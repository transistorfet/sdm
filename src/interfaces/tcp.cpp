/*
 * Module Name:		tcp.c
 * System Requirements:	Unix Sockets
 * Description:		TCP Network Interface Manager
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

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

#ifndef TCP_CONNECT_ATTEMPTS
#define TCP_CONNECT_ATTEMPTS		3
#endif

#ifndef TCP_LISTEN_QUEUE
#define TCP_LISTEN_QUEUE		5
#endif

MooObjectType moo_tcp_obj_type = {
	NULL,
	"tcp",
	(moo_type_create_t) moo_tcp_create
};

MooObject *moo_tcp_create(void)
{
	return(new MooTCP());
}


MooTCP::MooTCP()
{
	m_read_pos = 0;
	m_read_length = 0;
	memset(m_read_buffer, '\0', TCP_READ_BUFFER);
}

MooTCP::~MooTCP()
{
	this->disconnect();
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

	if ((m_rfd = socket(PF_INET, SOCK_STREAM, 0)) >= 0) {
		for (j = 0;host->h_addr_list[j];j++) {
			saddr.sin_addr = *((struct in_addr *) host->h_addr_list[j]);
			for (i = 0;i < TCP_CONNECT_ATTEMPTS;i++) {
				if (connect(m_rfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
					return(0);
			}
		}
	}
	return(-1);
}

int MooTCP::listen(int port)
{
	struct sockaddr_in saddr;

	memset(&saddr, '\0', sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	if (((m_rfd = socket(PF_INET, SOCK_STREAM, 0)) >= 0)
	    && (bind(m_rfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
	    && (listen(m_rfd, TCP_LISTEN_QUEUE) >= 0)) {
		return(0);
	}
	return(-1);
}

int accept(MooTCP *inter)
{
	int size;
	fd_set rd;
	struct sockaddr_in saddr;
	struct timeval timeout = { 0, 0 };

	this->disconnect();

	/** Make sure there is a connection waiting */
	FD_ZERO(&rd);
	FD_SET(m_rfd, &rd);
	if (select(m_rfd + 1, &rd, NULL, NULL, &timeout) <= 0)
		return(-1);

	size = sizeof(struct sockaddr_in);
	if ((m_rfd = accept(m_rfd, (struct sockaddr *) &saddr, &size))) {
		sdm_status("Accepted connection from %s", inet_ntoa(saddr.sin_addr));
		// TODO do anything you might want with the saddr
		return(0);
	}
	return(-1);
}

int MooTCP::disconnect()
{
	if (m_rfd <= 0)
		return;
	shutdown(m_rfd, 2);
	close(m_rfd);
	m_rfd = -1;
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
	for (i = 0;i < size;i++) {
		if (m_read_pos >= m_read_length)
			break;
		buffer[i] = m_read_buffer[m_read_pos++];
	}

	if (i < size) {
		FD_ZERO(&rd);
		FD_SET(m_rfd, &rd);
		if (select(m_rfd + 1, &rd, NULL, NULL, &timeout)
		    && ((j = recv(m_rfd, &buffer[i], size - i, 0)) > 0))
			i += j;
		if (j <= 0)
			return(-1);
	}
	if (m_read_pos < m_read_length)
		this->set_ready();
	return(i);
}

/**
 * Send the string of length size to the given network connection and
 * return the number of bytes written or -1 on error.
 */
int MooTCP::send(const char *msg, int size)
{
	int sent, count = 0;

	do {
		if ((sent = send(m_rfd, (void *) msg, size, 0)) < 0)
			return(-1);
		else if (!sent)
			return(0);
		count += sent;
	} while (count < size);
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
	if ((res = select(m_rfd + 1, &rd, NULL, NULL, &timeout)) < 0)
		return(-1);
	else if (res == 0)
		return(m_read_length - m_read_pos);

	if ((res = recv(m_rfd, &m_read_buffer[m_read_length], TCP_READ_BUFFER - m_read_length - 1, 0)) <= 0)
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



