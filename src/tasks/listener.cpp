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
	this->set_delete();
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
			throw MooException("No tcp type, %s", buffer);
	}
	else if (!strcmp(type, "task")) {
		data->read_string_entry(buffer, STRING_SIZE);
		if (!(m_ttype = moo_object_find_type(buffer, &moo_task_obj_type)))
			throw MooException("No task type, %s", buffer);
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
	MooTCP *newtcp;
	MooTask *newtask;

	// TODO we should probably make sure we don't get a loop somehow where we keep getting called and don't handle the request
	if (!(ready & IO_READY_READ)) {
		delete inter;
		return(-1);
	}
	if (!(newtcp = (MooTCP *) moo_make_object(m_itype))) {
		delete this;
		return(-1);
	}
	if ((m_inter->accept(newtcp) < 0)
	    || !(newtask = (MooTask *) moo_make_object(m_ttype))) {
		delete newtcp;
		return(-1);
	}
	newtask->bestow(newtcp);
	return(0);
}

int MooListener::purge(MooInterface *inter)
{
	if (inter != m_inter)
		return(-1);
	/// We assume that since we were called because the interface is already being deleted, in which case we don't want to delete too
	m_inter = NULL;
	if (!this->is_deleting())
		delete this;
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


