/*
 * Module Name:		tcp.c
 * System Requirements:	Unix Sockets
 * Description:		TCP Driver
 */


#include <string.h>
#include <stdarg.h>

#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/drivers/driver.h>
#include <sdm/drivers/tcp.h>

#include <sdm/code/code.h>

#ifndef TCP_CONNECT_ATTEMPTS
#define TCP_CONNECT_ATTEMPTS		3
#endif

MooObjectType moo_tcp_obj_type = {
	"tcp",
	typeid(MooTCP).name(),
	(moo_type_load_t) load_moo_tcp
};

static MooObjectHash *tcp_methods = new MooObjectHash();

int init_tcp(void)
{
	moo_object_register_type(&moo_tcp_obj_type);
	moo_load_tcp_methods(tcp_methods);
	return(0);
}

void release_tcp(void)
{
	tcp_methods = NULL;	/// Leave to the GC
	moo_object_deregister_type(&moo_tcp_obj_type);
}


MooObject *load_moo_tcp(MooDataFile *data)
{
	MooTCP *obj = new MooTCP();
	if (data)
		obj->read_data(data);
	return(obj);
}


MooTCP::MooTCP()
{
	m_read_pos = 0;
	m_read_length = 0;
	memset(m_read_buffer, '\0', TCP_READ_BUFFER);
	m_host = NULL;
}

MooTCP::~MooTCP()
{
	if (m_host) {
		moo_status("TCP: Disconnecting from %s", m_host->c_str());
		delete m_host;
	}
	this->disconnect();
}

int MooTCP::read_entry(const char *type, MooDataFile *data)
{
	return(MOO_NOT_HANDLED);
}


int MooTCP::write_data(MooDataFile *data)
{
	return(0);
}

MooObject *MooTCP::access_method(const char *name, MooObject *value)
{
	if (value)
		throw moo_permissions;
	return(tcp_methods->get(name));
}

int MooTCP::connect(const char *addr, int port)
{
	int i, j;
	struct hostent *host;
	struct sockaddr_in saddr;

	if (!(host = gethostbyname(addr)))
		return(-1);
	memset(&saddr, '\0', sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	if ((m_rfd = ::socket(PF_INET, SOCK_STREAM, 0)) >= 0) {
		for (j = 0;host->h_addr_list[j];j++) {
			saddr.sin_addr = *((struct in_addr *) host->h_addr_list[j]);
			for (i = 0;i < TCP_CONNECT_ATTEMPTS;i++) {
				if (::connect(m_rfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
					return(0);
			}
		}
	}
	return(-1);
}

void MooTCP::disconnect()
{
	if (m_rfd <= 0)
		return;
	::shutdown(m_rfd, 2);
	::close(m_rfd);
	m_rfd = -1;
	m_read_pos = 0;
	m_read_length = 0;
	this->clear_state();
}

int MooTCP::check_ready(int ready)
{
	fd_set rd;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&rd);
	FD_SET(m_rfd, &rd);
	if (::select(m_rfd + 1, &rd, NULL, NULL, &timeout)) {
		this->set_ready();
		return(1);
	}
	else {
		this->set_not_ready();
		return(0);
	}
}


int MooTCP::read(char *data, int len)
{
	int res;

	if ((res = this->receive(data, len - 1)) < 0)
		return(res);
	data[res] = '\0';
	return(res);
}

int MooTCP::write(const char *data)
{
	return(this->send(data, strlen(data)));
}


/**
 * Receive the given number of bytes, store them in the given buffer
 * and return the number of bytes read or -1 on error or disconnect.
 */ 
int MooTCP::receive(char *data, int len)
{
	int i, j;
	fd_set rd;
	struct timeval timeout = { 0, 0 };

	this->set_not_ready();
	for (i = 0; i < len; i++) {
		if (m_read_pos >= m_read_length)
			break;
		data[i] = m_read_buffer[m_read_pos++];
	}

	if (i < len) {
		FD_ZERO(&rd);
		FD_SET(m_rfd, &rd);
		if (::select(m_rfd + 1, &rd, NULL, NULL, &timeout)
		    && ((j = ::recv(m_rfd, &data[i], len - i, 0)) > 0))
			i += j;
		if (j <= 0)
			throw moo_closed;
	}
	if (m_read_pos < m_read_length)
		this->set_ready();
	return(i);
}

int MooTCP::receive(char *data, int len, char delim)
{
	int i, j = 0;

	if (this->recv_to_buffer() < 0)
		throw moo_closed;
	for (i = m_read_pos; i < m_read_length; i++) {
		if (m_read_buffer[i] == delim) {
			data[j] = '\0';
			this->read_pos(i + 1);
			return(j);
		}
		data[j++] = m_read_buffer[i];
	}
	/// We didn't receive a delim character so we'll act like we didn't read anything
	return(0);
}



/**
 * Send the string of length size to the given network connection and
 * return the number of bytes written or -1 on error.
 */
int MooTCP::send(const char *data, int len)
{
	int sent, count = 0;

	if (len == -1)
		len = strlen(data);
	do {
		if ((sent = ::send(m_rfd, (void *) data, len, 0)) < 0)
			return(-1);
		else if (!sent)
			return(0);
		count += sent;
	} while (count < len);
	return(count);
}


/**
 * Receive data directly into the read buffer and return the number of bytes
 * read or -1 on error or disconnect.
 */ 
int MooTCP::recv_to_buffer()
{
	int res;
	fd_set rd;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&rd);
	FD_SET(m_rfd, &rd);
	if ((res = ::select(m_rfd + 1, &rd, NULL, NULL, &timeout)) < 0)
		return(-1);
	else if (res == 0)
		return(m_read_length - m_read_pos);

	if ((res = ::recv(m_rfd, &m_read_buffer[m_read_length], TCP_READ_BUFFER - m_read_length - 1, 0)) <= 0)
		return(-1);
	res += m_read_length;
	m_read_length = res;
	m_read_buffer[res] = '\0';
	this->set_ready();
	return(res - m_read_pos);
}

/**
 * Set the read buffer to contain the given data.
 */
int MooTCP::load_buffer(const char *data, int len)
{
	if (len > TCP_READ_BUFFER)
		len = TCP_READ_BUFFER;
	strncpy(m_read_buffer, data, len);
	m_read_length = len;
	m_read_pos = 0;
	if (len)
		this->set_ready();
	else
		this->set_not_ready();
	return(len);
}

/**
 * Set the position in the read buffer that has been read to the given value or
 * if the position is greater than or equal to the length of the read buffer,
 * clear the read buffer.  The position of the read pointer is returned.
 */
int MooTCP::read_pos(int pos)
{
	if (pos >= m_read_length) {
		this->set_not_ready();
		m_read_pos = 0;
		m_read_length = 0;
	}
	else {
		this->set_ready();
		m_read_pos = pos;
	}
	return(m_read_pos);
}

/**
 * Clear the read buffer.
 */
void MooTCP::clear_buffer()
{
	this->set_not_ready();
	m_read_pos = 0;
	m_read_length = 0;
}

int MooTCP::wait_for_data()
{
	if (m_task)
		throw MooException("TCP: Another task is waiting, aborting.");
	m_task = MooTask::current_task();
	if (!m_task)
		throw MooException("TCP: No task currently running, aborting wait.");

	/// If data is available, then return right away
	if (this->check_ready(IO_READY_READ))
		return(0);
	/// Otherwise suspend the currently executing frame
	throw MooCodeFrameSuspend();
}

int MooTCP::handle(int ready)
{
	if (m_task) {
		m_task->schedule(0);
		m_task = NULL;
	}
	return(0);
}


/**********************
 * TCP Object Methods *
 **********************/

static int tcp_wait(MooCodeFrame *frame, MooObjectArray *args)
{
	MooTCP *m_this;

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(m_this = dynamic_cast<MooTCP *>(args->get(0))))
		throw moo_method_object;
	m_this->wait_for_data();
	return(0);
}

static int tcp_print(MooCodeFrame *frame, MooObjectArray *args)
{
	MooTCP *m_this;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(m_this = dynamic_cast<MooTCP *>(args->get(0))))
		throw moo_method_object;
	// TODO wtf? also, should we have a printf format instead?
	MooObject::format(buffer, LARGE_STRING_SIZE, frame->env(), args->get_string(1));
	//moo_status("TCP: SEND DEBUG: %s", buffer);
	m_this->send(buffer);
	return(0);
}


void moo_load_tcp_methods(MooObjectHash *env)
{
	env->set("wait", new MooFuncPtr(tcp_wait));
	env->set("print", new MooFuncPtr(tcp_print));
}


