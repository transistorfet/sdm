/*
 * Module Name:		server.c
 * System Requirements:	Unix Sockets
 * Description:		TCP Server Driver
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
#include <sdm/drivers/server.h>

#ifndef TCP_LISTEN_QUEUE
#define TCP_LISTEN_QUEUE		5
#endif

MooObjectType moo_server_obj_type = {
	"server",
	typeid(MooServer).name(),
	(moo_type_load_t) load_moo_server
};

MooObject *load_moo_server(MooDataFile *data)
{
	MooServer *obj = new MooServer();
	if (data)
		obj->read_data(data);
	return(obj);
}


MooServer::MooServer()
{
	m_port = 0;
}

MooServer::~MooServer()
{
	moo_status("TCP: shutting down server on port %d", m_port);
	this->disconnect();
}

int MooServer::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "port"))
		m_port = data->read_integer_entry();
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}


int MooServer::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_integer_entry("port", m_port);
	return(0);
}

int MooServer::handle(int ready)
{
	MooCodeFrame *frame;

	if (!(ready & IO_READY_READ))
		return(-1);
	frame = new MooCodeFrame();
	// TODO how will you start the new task? You will probably want to run a method on the server thing but should it be selectable?
	// 	how will it be set? (since we write our data though, it only has to be set once, at creation)
	frame->push_code("(print \"What the fuck do I do\")");
	frame->schedule(0);
	frame = NULL;	/// The task will be destroyed when it finishes executing
	return(0);
}


int MooServer::listen(int port)
{
	struct sockaddr_in saddr;

	memset(&saddr, '\0', sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	if (((m_rfd = ::socket(PF_INET, SOCK_STREAM, 0)) >= 0)
	    && (::bind(m_rfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
	    && (::listen(m_rfd, TCP_LISTEN_QUEUE) >= 0)) {
		moo_status("TCP: Listening on port %d", port);
		return(0);
	}
	return(-1);
}

MooTCP *MooServer::receive_connection()
{
	int size;
	fd_set rd;
	MooTCP *driver;
	struct sockaddr_in saddr;
	struct timeval timeout = { 0, 0 };

	driver = new MooTCP();

	/// Make sure there is a connection waiting
	FD_ZERO(&rd);
	FD_SET(m_rfd, &rd);
	if (::select(m_rfd + 1, &rd, NULL, NULL, &timeout) <= 0)
		return(-1);

	size = sizeof(struct sockaddr_in);
	if ((driver->m_rfd = ::accept(m_rfd, (struct sockaddr *) &saddr, (socklen_t *) &size)) > 0) {
		driver->m_host = new std::string(inet_ntoa(saddr.sin_addr));
		moo_status("TCP: Accepted connection from %s", inter->m_host->c_str());
		return(driver);
	}
	delete driver;
	return(NULL);
}

void MooServer::disconnect()
{
	if (m_rfd <= 0)
		return;
	::shutdown(m_rfd, 2);
	::close(m_rfd);
	m_rfd = -1;
	this->clear_state();
}


