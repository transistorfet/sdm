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
#include <sdm/code/code.h>

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
	m_env = NULL;
	// TODO we should pass parent to the constructor, but MooObjectHash doesn't yet support nested tables
	this->env(new MooObjectHash());
	m_return = NULL;
}

MooCodeFrame::~MooCodeFrame()
{
	MOO_DECREF(m_stack);
	MOO_DECREF(m_env);
	MOO_DECREF(m_return);
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

int MooCodeFrame::add_block(MooArgs *args, MooCodeExpr *expr)
{
	return(m_stack->push(new MooCodeEventEvalBlock(m_env, args, expr)));
}

int MooCodeFrame::run(int level)
{
	int res;
	MooCodeEvent *event;

	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	if (level < 0)
		level = 0;
	while (m_stack->last() >= level) {
		if (!(event = m_stack->pop()))
			continue;
		try {
			this->env(event->env());
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

int MooCodeFrame::eval(const char *code, MooArgs *args)
{
	MooCodeExpr *expr;

	if (*code == '\0')
		return(-1);
	try {
		MooCodeParser parser;
		expr = parser.parse(code);
		// TODO temporary
		MooCodeParser::print(expr);
	}
	catch (MooException e) {
		moo_status("%s", e.get());
		return(-1);
	}
	return(this->call(expr, args));
}

int MooCodeFrame::call(MooCodeExpr *expr, MooArgs *args)
{
	int ret;
	MooObjectHash *env;

	// TODO args can be NULL here, don't forget.

	// TODO add args to the environment (as MooArgs, or as something else?)
	//env = frame.env();
	//env->set("args", args);
	//env->set("parent", new MooThingRef(m_thing));
	this->add_block(args, expr);
	ret = this->run(m_stack->last());
	if (args)
		MOO_INCREF(args->m_result = m_return);
	return(ret);
}

int MooCodeFrame::call(MooCodeExpr *expr, MooArgs *parent, int num_params, ...)
{
	MooArgs *args;

	if (parent)
		args = new MooArgs(parent);
	else
		args = new MooArgs();

	// TODO go through all the values on the stack and push them onto a new MooArgs

	return(this->call(expr, args));
}

void MooCodeFrame::set_return(MooObject *obj)
{
	MOO_DECREF(m_return);
	m_return = MOO_INCREF(obj);
}

void MooCodeFrame::env(MooObjectHash *env)
{
	MOO_DECREF(m_env);
	MOO_INCREF(m_env = env);
}


