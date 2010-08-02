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

}

MooListener::~MooListener()
{
	if (m_inter)
		delete m_inter;
}

int MooListener::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "load")) {
		// TODO what could you read in here?
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}


int MooListener::write_data(MooDataFile *data)
{
	// TODO what could you write here?
	//data->write_begin_entry("load");
	//data->write_attrib("ref", m_filename);
	//data->write_end_entry();
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


	char buffer[LARGE_STRING_SIZE];

	inter->read(buffer, LARGE_STRING_SIZE);
	printf("%s", buffer);

	return(0);
}


