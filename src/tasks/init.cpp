/*
 * Task Name:	init.cpp
 * Description:	Init Task
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/task.h>
#include <sdm/tasks/init.h>


MooObjectType moo_init_obj_type = {
	"init",
	typeid(MooInit).name(),
	(moo_type_load_t) load_moo_init
};

MooObject *load_moo_init(MooDataFile *data)
{
	MooInit *obj = new MooInit();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooInit::MooInit()
{

}

MooInit::~MooInit()
{
	this->set_delete();
	//if (m_inter)
	//	delete m_inter;
}

int MooInit::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

/*
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
*/
	return(MOO_HANDLED);
}


int MooInit::write_data(MooDataFile *data)
{
/*
	data->write_integer_entry("port", m_port);
	if (m_itype)
		data->write_string_entry("type", m_itype->m_name);
	if (m_ttype)
		data->write_string_entry("task", m_ttype->m_name);
*/
	return(0);
}

int MooInit::initialize()
{

	return(0);
}

int MooInit::idle()
{
	return(0);
}

int MooInit::release()
{

	return(0);
}


