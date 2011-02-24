/*
 * Name:	frame.cpp
 * Description:	MooCode Frame
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
#include "event.h"
#include "frame.h"

MooObjectType moo_code_frame_obj_type = {
	&moo_object_obj_type,
	"frame",
	typeid(MooCodeFrame).name(),
	(moo_type_create_t) moo_code_frame_create
};


MooObject *moo_code_frame_create(void)
{
	return(new MooCodeFrame());
}

MooCodeFrame::MooCodeFrame(MooObjectHash *parent)
{
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	// TODO we should pass parent to the constructor, but MooObjectHash doesn't yet support nested tables
	m_env = new MooObjectHash();
	m_return = NULL;
}

MooCodeFrame::~MooCodeFrame()
{
	if (m_stack)
		delete m_stack;
	if (m_env)
		delete m_env;
	if (m_return)
		delete m_return;
}

int MooCodeFrame::read_entry(const char *type, MooDataFile *data)
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

int MooCodeFrame::write_data(MooDataFile *data)
{
	const char *name;

	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	//data->write_string_entry("params", this->params());
	return(0);
}

int MooCodeFrame::push_event(MooCodeEvent *event)
{
	return(m_stack->push(event));
}

int MooCodeFrame::add_block(MooCodeExpr *expr)
{
	return(m_stack->push(new MooCodeEventEvalBlock(m_env, expr)));
}


int MooCodeFrame::call(const char *name, MooCodeExpr *expr)
{
	// TODO add eval expr event
	// TODO add call function event (how are args passed from expr eval event to function call event?)
}

int MooCodeFrame::call(const char *name, MooArgs *args)
{
	// TODO look for name in primatives/library functions list
	// TODO add eval event if found
	// TODO find the action with the given name (on what object??)
	// TODO call action (or do we only set an event to call the action?)
}

int MooCodeFrame::eval(MooCodeExpr *expr)
{

}


int MooCodeFrame::run(int level)
{
	int res;
	MooCodeEvent *event;

	// TODO should level be relative?  That way you always call run(0) and it runs to the starting point
	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	if (level < 0)
		level = 0;
	while (m_stack->last() > level) {
		if (!(event = m_stack->pop()))
			continue;
		try {
			// TODO do we set the frame env to the event's env before doing the event?
			res = event->do_event(this);
			if (res < 0)
				return(res);
			delete event;
		}
		catch (MooException e) {
			// TODO what do we do if there's an error?
			return(-1);
		}
		catch (...) {

		}
	}
	return(0);
}

void MooCodeFrame::set_return(MooObject *obj)
{
	if (m_return)
		delete m_return;
	m_return = obj;
}

void MooCodeFrame::env(MooObjectHash *env)
{
	// TODO m_env owns a reference to an environment.  The appropriate environment for an event is stored in the event
	//	itself, so this function will be called when an event is being executed.  When an environment is created
	//	for a function call or action or something, it's only reference will be owned by the event which will thus
	//	destroy the environment when the event is destroyed (unless a new reference is created during the event's
	//	execution)...  Should the event change m_env here before it finishes executing?
//	if (m_env)
//		delete m_env;
	m_env = env;
}


