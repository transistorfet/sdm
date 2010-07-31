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

int MooListener::handle(MooInterface *inter)
{
	// TODO this will create an object of type m_itype and pass it to m_inter->accept()
	// TODO create a new process of type m_ptype and set it up so that the new interface calls it and it does everythnig with
	//	that interface (perhaps call proc->take(inter) which sets it's own m_inter field (or array of interfaces) and then
	//	calls inter->set_task(this)
}


