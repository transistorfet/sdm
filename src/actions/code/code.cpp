/*
 * Name:	code.cpp
 * Description:	MooCode
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
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/actions/action.h>
#include <sdm/actions/code/code.h>

#include "expr.h"
#include "frame.h"
#include "parser.h"

MooObjectType moo_code_obj_type = {
	&moo_action_obj_type,
	"moocode",
	typeid(MooCode).name(),
	(moo_type_create_t) moo_code_create
};

static MooObjectHash *global_env = NULL;

extern int moo_load_code_basic(MooObjectHash *env);

int init_moo_code(void)
{
	if (moo_object_register_type(&moo_code_obj_type) < 0)
		return(-1);
	global_env = new MooObjectHash();
	if (global_env)
		return(1);
	moo_load_code_basic(global_env);
	return(0);
}

void release_moo_code(void)
{
	if (!global_env)
		return;
	delete global_env;
	global_env = NULL;
	moo_object_deregister_type(&moo_code_obj_type);
}

MooObject *moo_code_create(void)
{
	return(new MooCode());
}

MooCode::MooCode(MooCodeExpr *code, const char *params, const char *name, MooThing *thing) : MooAction(name, thing)
{
	m_code = code;
	if (params)
		this->params(params);
}

int MooCode::read_entry(const char *type, MooDataFile *data)
{
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
	return(MOO_HANDLED);
}

int MooCode::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	data->write_string_entry("params", this->params());
	return(0);
}

int MooCode::do_action(MooThing *thing, MooArgs *args)
{
	int ret;
	MooObjectHash *env;
	MooCodeFrame frame;

	env = frame.env();
	// TODO add args to the environment (as MooArgs, or as something else?)
	//env->set("args", args);
	env->set("parent", new MooThingRef(thing));
	frame.add_block(m_code);
	ret = frame.run();
	args->m_result = frame.get_return();
	return(ret);

	/****

	we need a thread in order to execute the code we have.  If a thread is also a task, we can't be sure if the
	current task is a thread (it probably isn't in fact).  We definitely don't want to make a new thread for
	each action call, but then again, if it was lightweight enough, I suppose we could, in which case it would
	be more like a stack frame than a thread.  We also don't want a dedicated thread for each action, which not
	only would mean a lot of threads sitting around, but would make it quite complicated during execution of
	nested action calls and so on.

	A thread should be called a frame instead.  It can be stored in each action or allocated when an action is
	executed.  A reference to a global environment could be included in the action itself.  The current task
	will "run" the frame/thread.


	*/
}

int MooCode::set(const char *code)
{
	MooCodeParser parser;

	m_code = parser.parse(code);
	return(0);
}


