/*
 * Name:	thread.cpp
 * Description:	MooCode Thread
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/things/thing.h>
#include <sdm/actions/action.h>
#include <sdm/actions/code/code.h>

#include "expr.h"
#include "thread.h"

MooObjectType moo_code_thread_obj_type = {
	&moo_object_obj_type,
	"thread",
	typeid(MooCodeThread).name(),
	(moo_type_create_t) moo_code_thread_create
};


MooObject *moo_code_thread_create(void)
{
	return(new MooCodeThread());
}

MooCodeThread::MooCodeThread()
{
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	m_return = NULL;
}

MooCodeThread::~MooCodeThread()
{
	if (m_stack)
		delete m_stack;
	if (m_return)
		delete m_return;
}

int MooCodeThread::read_entry(const char *type, MooDataFile *data)
{
/*
	if (!strcmp(type, "code")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->set(buffer);
	}
	else if (!strcmp(type, "params")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->params(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
*/
	return(MOO_HANDLED);
}

int MooCodeThread::write_data(MooDataFile *data)
{
	const char *name;

	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	//data->write_string_entry("params", this->params());
	return(0);
}

int MooCodeThread::push_event(MooCodeEvent *event)
{
	return(m_stack->push(event));
}

int MooCodeThread::add_block(MooCodeExpr *expr)
{
	MooCodeEvent *event;

	event = new MooCodeEventEvalExpr(expr);
	return(m_stack->push(event));
}


int MooCodeThread::call(const char *name, MooCodeExpr *expr)
{
	// TODO add eval expr event
	// TODO add call function event (how are args passed from expr eval event to function call event?)
}

int MooCodeThread::call(const char *name, MooArgs *args)
{
	// TODO look for name in primatives/library functions list
	// TODO add eval event if found
	// TODO find the action with the given name (on what object??)
	// TODO call action (or do we only set an event to call the action?)
}

int MooCodeThread::eval(MooCodeExpr *expr)
{

}


int MooCodeThread::run(int level)
{
	int res;
	MooCodeEvent *event;

	// TODO should level be relative?  That way you always call run(0) and it runs to the starting point
	// TODO add an event counter in the thread and also take a max events param or something, such that
	//	a thread gets a limited time slice...
	if (level < 0)
		level = 0;
	while (m_stack->last() > level) {
		if (!(event = m_stack->pop()))
			continue;
		// TODO how does this work?
		res = event->do_event(this);
		if (res < 0)
			return(res);
	}
	return(0);
}

int MooCodeThread::run_all(int cycles)
{
	// TODO run all threads for a max of cycles/num_threads
}

void MooCodeThread::set_return(MooObject *obj)
{
	if (m_return)
		delete m_return;
	m_return = obj;
}


