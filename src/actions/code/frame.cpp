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

extern MooObjectHash *global_env;

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
	//const char *name;

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

int MooCodeFrame::run(int level)
{
	int res;
	MooCodeEvent *event;

	// TODO should level be relative?  That way you always call run(0) and it runs to the starting point
	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	if (level < 0)
		level = 0;
	while (m_stack->last() >= level) {
		if (!(event = m_stack->pop()))
			continue;
		try {
			m_env = event->m_env;
			res = event->do_event(this);
			delete event;
			if (res < 0)
				return(res);
		}
		catch (MooException e) {
			delete event;
			// TODO this should be printed to the user (probably instead of rather than in addition to the status)
			moo_status("CODE: %s", e.get());
			return(-1);
		}
		catch (...) {
			moo_status("CODE: Unknown error occurred");
			return(-1);
		}
	}
	return(0);
}

void MooCodeFrame::set_return(MooObject *obj)
{
	if (m_return)
		delete m_return;
	m_return = MOO_INCREF(obj);
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

MooObject *MooCodeFrame::resolve(const char *name, MooArgs *args)
{
	int i;
	MooObject *obj;
	char *action = NULL;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	if ((action = strchr(buffer, ':'))) {
		*action = '\0';
		action++;
	}

	// TODO temporary
	if (!(obj = m_env->get(name, NULL)) && !(obj = global_env->get(name, NULL)))
		return(NULL);
	return(obj);

	// TODO possible:  #124, me, variable, thing.prop, thing:action, #124.prop, etc
	// TODO do variable lookup, use frame to find the evironment/table to look up
	//	do a lookup on the frame env, then a lookup on global_env, then give up (since we don't have nested envs)

	// TODO this might require a bunch of rewriting of the thing reference/string expander code
	// if contains ':', then resolve lhs to a thing pointer, and look up the action (???)
	// if contains a '.', then do all that processing

	// if found in local environment, return value
	// if found in global environment, return value
}

