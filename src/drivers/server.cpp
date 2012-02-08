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
#include <sdm/objs/mutable.h>
#include <sdm/drivers/driver.h>
#include <sdm/drivers/tcp.h>
#include <sdm/drivers/server.h>

#include <sdm/code/code.h>

#ifndef TCP_LISTEN_QUEUE
#define TCP_LISTEN_QUEUE		5
#endif

static MooObjectHash *server_methods = new MooObjectHash();


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

/************************
 * MooServer Definition *
 ************************/

MooServer::MooServer()
{
	m_port = 0;
	m_obj = NULL;
	m_method = NULL;
}

MooServer::~MooServer()
{
	moo_status("TCP: shutting down server on port %d", m_port);
	this->disconnect();
	if (m_method)
		delete m_method;
}

MooObject *MooServer::access_method(const char *name, MooObject *value)
{
	if (value)
		throw moo_permissions;
	return(server_methods->get(name));
}

int MooServer::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "port"))
		m_port = data->read_integer_entry();
	else if (!strcmp(type, "object")) {
		moo_id_t id = data->read_integer_entry();
		// TODO this can't be a static type check but the lookup function should really return a mutable type =/
		if (!(m_obj = dynamic_cast<MooMutable *>(MooMutable::lookup(id))))
			moo_status("SERVER: Error loading callback object id, %d", id);
	}
	else if (!strcmp(type, "method")) {
		char buffer[STRING_SIZE];
		data->read_string_entry(buffer, STRING_SIZE);
		m_method = new std::string(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}


int MooServer::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_integer_entry("port", m_port);
	if (m_obj)
		data->write_integer_entry("object", m_obj->id());
	if (m_method)
		data->write_string_entry("method", m_method->c_str());
	return(0);
}

int MooServer::handle(int ready)
{
	MooTCP *driver;
	MooCodeFrame *frame;

	if (!(ready & IO_READY_READ))
		return(-1);
	if (!(driver = this->receive_connection()))
		return(-1);
	if (!m_method || !m_obj) {
		delete driver;
		return(-1);
	}

	moo_status("SERVER: DEBUG: Calling method %s", m_method->c_str());
	frame = new MooCodeFrame();
	frame->push_method_call(m_method->c_str(), m_obj, driver);
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
		return(NULL);

	size = sizeof(struct sockaddr_in);
	if ((driver->m_rfd = ::accept(m_rfd, (struct sockaddr *) &saddr, (socklen_t *) &size)) > 0) {
		driver->m_host = new std::string(inet_ntoa(saddr.sin_addr));
		moo_status("TCP: Accepted connection from %s", driver->m_host->c_str());
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

int MooServer::set_callback(MooMutable *obj, const char *method)
{
	if (!obj || !method)
		return(-1);
	if (m_method)
		delete m_method;
	m_obj = obj;
	m_method = new std::string(method);
	return(0);
}

/*************************
 * Server Object Methods *
 *************************/

static int server_make_server(MooCodeFrame *frame, MooObjectArray *args)
{
	int port = 4000;
	MooServer *server;

	if (args->last() >= 1)
		throw moo_args_mismatched;
	else if (args->last() == 0)
		port = args->get_integer(0);
	server = new MooServer();
	server->listen(port);
	frame->set_return(server);
	return(0);
}

static int server_set_callback(MooCodeFrame *frame, MooObjectArray *args)
{
	MooServer *m_this;
	MooMutable *obj;
	const char *method;

	if (args->last() != 2)
		throw moo_args_mismatched;
	if (!(m_this = dynamic_cast<MooServer *>(args->get(0))))
		throw moo_method_object;
	if (!(obj = dynamic_cast<MooMutable *>(args->get(1))))
		throw MooException("arg 1: Invalid type; expected mutable object.");
	if (!(method = args->get_string(2)))
		throw MooException("arg 2: Invalid type; expected stringifiable object.");
	m_this->set_callback(obj, method);
	return(0);
}

void moo_load_server_methods(MooObjectHash *env)
{
	env->set("set_callback", new MooFuncPtr(server_set_callback));
}

int init_server(void)
{
	moo_object_register_type(&moo_server_obj_type);
	moo_load_server_methods(server_methods);
	global_env->set("make-server", new MooFuncPtr(server_make_server));
	return(0);
}

void release_server(void)
{
	server_methods = NULL;	/// Leave to the GC
	moo_object_deregister_type(&moo_server_obj_type);
}



