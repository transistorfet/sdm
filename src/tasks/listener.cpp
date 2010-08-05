/*
 * Task Name:	listener.cpp
 * Description:	Listening Task
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
#include <sdm/tasks/listener.h>


MooObjectType moo_listener_obj_type = {
	&moo_task_obj_type,
	"listener",
	typeid(MooListener).name(),
	(moo_type_create_t) moo_listener_create
};

MooObject *moo_listener_create(void)
{
	return(new MooListener());
}

MooListener::MooListener()
{
	m_port = -1;
	m_itype = NULL;
	m_ttype = NULL;
	m_inter = NULL;
}

MooListener::~MooListener()
{
	if (m_inter)
		delete m_inter;
}

int MooListener::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

	if (!strcmp(type, "port")) {
		m_port = data->read_integer_entry();
		this->listen(m_port);
	}
	else if (!strcmp(type, "type")) {
		data->read_string_entry(buffer, STRING_SIZE);
		if (!(m_itype = moo_object_find_type((*buffer != '\0') ? buffer : "tcp", &moo_tcp_obj_type)))
			return(-1);
	}
	else if (!strcmp(type, "task")) {
		data->read_string_entry(buffer, STRING_SIZE);
		if (!(m_ttype = moo_object_find_type(buffer, &moo_task_obj_type)))
			return(-1);
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}


int MooListener::write_data(MooDataFile *data)
{
	data->write_integer_entry("port", m_port);
	if (m_itype)
		data->write_string_entry("type", m_itype->m_name);
	if (m_ttype)
		data->write_string_entry("task", m_ttype->m_name);
	return(0);
}

int MooListener::initialize()
{

	return(0);
}

int MooListener::idle()
{

	return(0);
}

int MooListener::release()
{

	return(0);
}

int MooListener::handle(MooInterface *inter, int ready)
{
	// TODO this will create an object of type m_itype and pass it to m_inter->accept()
	// TODO create a new process of type m_ptype and set it up so that the new interface calls it and it does everythnig with
	//	that interface (perhaps call proc->take(inter) which sets it's own m_inter field (or array of interfaces) and then
	//	calls inter->set_task(this)

	MooTCP *newtcp;
	MooTask *newtask;

	printf("HERE\n");

	if (!(ready & IO_READY_READ))
		return(-1);
	if (!(newtcp = (MooTCP *) moo_make_object(m_itype)))
		return(-1);
	if (!(m_inter->accept(newtcp))
	    || !(newtask = (MooTask *) moo_make_object(m_ttype))) {
		delete newtcp;
		return(-1);
	}
	newtask->bestow(newtcp);
	return(0);
}

int MooListener::listen(int port)
{
	// TODO is this the correct way to close the existing server connection?
	if (m_inter)
		delete m_inter;
	m_inter = new MooTCP();
	m_inter->set_task(this);
	m_inter->listen(port);
	return(0);
}


