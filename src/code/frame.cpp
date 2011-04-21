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
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/tasks/task.h>

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
	m_exception = MooException();
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	m_env = NULL;
	this->env(new MooObjectHash(parent));
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

int MooCodeFrame::push_block(MooObjectHash *env, MooCodeExpr *expr, MooArgs *args)
{
	return(m_stack->push(new MooCodeEventEvalBlock(env, args, expr)));
}

int MooCodeFrame::push_call(MooObjectHash *env, MooObject *func, MooArgs *args)
{
	args->m_args->unshift(func);
	return(m_stack->push(new MooCodeEventCallExpr(env, args)));
}

int MooCodeFrame::push_code(const char *code, MooArgs *args)
{
	MooCodeExpr *expr;

	if (*code == '\0')
		return(-1);
	MooCodeParser parser;
	expr = parser.parse(code);
	// TODO temporary
	MooCodeParser::print(expr);
	return(this->push_block(m_env, expr, args));
}

int MooCodeFrame::push_debug(const char *msg, ...)
{
	va_list va;
	char buffer[STRING_SIZE];

	va_start(va, msg);
	vsnprintf(buffer, STRING_SIZE, msg, va);
	return(m_stack->push(new MooCodeEventDebug(buffer)));
}

int MooCodeFrame::run(int level)
{
	int res;
	MooObjectHash *base;
	MooCodeEvent *event;

	base = m_env;
	m_exception = MooException("");
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
		}
		catch (MooException e) {
			int line, col;

			this->env(base);
			if (e.severity() == E_FATAL)
				throw e;
			m_stack->push(event);
			if (this->linecol(line, col))
				m_exception = MooException(e.severity(), "(%d, %d): %s", line, col, e.get());
			else
				m_exception = e;
			throw m_exception;
		}
		catch (...) {
			this->env(base);
			m_exception = MooException("Unknown error occurred");
			throw m_exception;
		}
		delete event;
		if (res < 0)
			return(res);
	}
	this->env(base);
	return(0);
}

int MooCodeFrame::eval(const char *code, MooArgs *args)
{
	int level;

	try {
		level = m_stack->last();
		this->push_code(code, args);
		return(this->run(level));
	}
	catch (MooException e) {
		moo_status("CODE: %s", e.get());
		return(-1);
	}
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

int MooCodeFrame::linecol(int &line, int &col)
{
	int i;
	MooCodeEvent *cur;

	for (i = m_stack->last(); i >= 0; i--) {
		if (!(cur = m_stack->get(i)))
			return(0);
		if (cur->linecol(line, col))
			return(1);
	}
	return(0);
}

void MooCodeFrame::print_stack()
{
	MooCodeEvent *event;

	for (int i = m_stack->last(); i >= 0; i--) {
		event = m_stack->get(i);
		event->print_debug();
	}
}

