/*
 * Task Name:	rpc-server.cpp
 * Description:	RPC Server Task
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/task.h>
#include <sdm/tasks/rpc-server.h>


MooObjectType moo_rpc_server_obj_type = {
	&moo_task_obj_type,
	"rpc-server",
	typeid(MooRPCServer).name(),
	(moo_type_create_t) moo_rpc_server_create
};

MooObject *moo_rpc_server_create(void)
{
	return(new MooRPCServer());
}

MooRPCServer::MooRPCServer()
{

}

MooRPCServer::~MooRPCServer()
{
	if (m_inter)
		delete m_inter;
}

int MooRPCServer::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "port")) {
		m_port = data->read_integer_entry();
		//this->listen(m_port);
	}
	else if (!strcmp(type, "type")) {
		// TODO read in string and find type
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}


int MooRPCServer::write_data(MooDataFile *data)
{
	data->write_integer_entry("port", m_port);
	if (m_itype)
		data->write_string_entry("type", m_itype->m_name);
	if (m_ttype)
		data->write_string_entry("task", m_ttype->m_name);
	return(0);
}

int MooRPCServer::initialize()
{

	return(0);
}

int MooRPCServer::idle()
{

	return(0);
}

int MooRPCServer::release()
{

	return(0);
}

int MooRPCServer::handle(MooInterface *inter, int ready)
{

	return(0);
}

int MooRPCServer::bestow(MooInterface *inter)
{
	// TODO do we need to do any other special stuff before disconnecting the old socket
	if (m_inter)
		delete m_inter;
	m_inter = (MooTCP *) inter;
	m_inter->set_task(this);
	return(0);
}

// TODO Ultimately this task will just read in RPC calls (possibly in XML)

