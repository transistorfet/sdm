/*
 * Name:	frame.cpp
 * Description:	MooCode Frame
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/tasks/task.h>

#include <sdm/things/thing.h>
#include <sdm/code/code.h>

MooObjectType moo_code_frame_obj_type = {
	&moo_object_obj_type,
	"frame",
	typeid(MooCodeFrame).name(),
	(moo_type_make_t) make_moo_code_frame
};

extern MooObjectHash *global_env;

MooObject *make_moo_code_frame(MooDataFile *data)
{
	MooCodeFrame *obj = new MooCodeFrame();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooCodeFrame::MooCodeFrame(MooObjectHash *parent)
{
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	m_return = NULL;
	m_exception = NULL;
	m_env = NULL;
	this->env(new MooObjectHash(parent));
}

MooCodeFrame::~MooCodeFrame()
{
	MOO_DECREF(m_stack);
	MOO_DECREF(m_env);
	MOO_DECREF(m_return);
	if (m_exception)
		delete m_exception;
}

int MooCodeFrame::read_entry(const char *type, MooDataFile *data)
{
	moo_status("DATA: Attempting to read unreadable type: MooCodeFrame (%x)", this);
	return(0);
}

int MooCodeFrame::write_data(MooDataFile *data)
{
	moo_status("DATA: Attempting to write unwritable type: MooCodeFrame (%x)", this);
	return(0);
}

int MooCodeFrame::push_event(MooCodeEvent *event)
{
	return(m_stack->push(event));
}

int MooCodeFrame::push_block(MooObjectHash *env, MooCodeExpr *expr)
{
	return(m_stack->push(new MooCodeEventEvalBlock(env, expr)));
}

int MooCodeFrame::push_call(MooObjectHash *env, MooObject *func, MooArgs *args)
{
	args->m_args->unshift(func);
	return(m_stack->push(new MooCodeEventCallFunc(env, args)));
}

int MooCodeFrame::push_code(const char *code)
{
	MooCodeExpr *expr;

	if (*code == '\0')
		return(-1);
	MooCodeParser parser;
	expr = parser.parse(code);
	// TODO temporary
	MooCodeParser::print(expr);
	return(this->push_block(m_env, expr));
}

int MooCodeFrame::push_debug(const char *msg, ...)
{
	va_list va;
	char buffer[STRING_SIZE];

	va_start(va, msg);
	vsnprintf(buffer, STRING_SIZE, msg, va);
	return(m_stack->push(new MooCodeEventDebug(buffer)));
}

int MooCodeFrame::run()
{
	MooObjectHash *base;
	MooCodeEvent *event;

	base = m_env;
	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	while (m_stack->last() >= 0) {
		if (!(event = m_stack->pop()))
			continue;

		if (m_exception) {
			delete m_exception;
			m_exception = NULL;
		}

		try {
			this->env(event->env());
			event->do_event(this);
		}
		catch (MooException e) {
			int line, col;

			if (e.is_fatal())
				throw e;
			this->linecol(line, col);
			m_exception = new MooException(e.type(), "(%d, %d): %s", line, col, e.get());
		}
		catch (...) {
			m_exception = new MooException("Unknown error occurred");
		}

		if (m_exception) {
			m_stack->push(event);
			if (!this->handle_exception()) {
				this->env(base);
				throw *m_exception;
			}
		}
		else
			delete event;
	}
	this->env(base);
	return(0);
}

int MooCodeFrame::handle_exception()
{
	MooCodeEvent *event;
	MooCodeEventCatch *handler;

	for (int i = m_stack->last(); i >= 0; i--) {
		if ((handler = dynamic_cast<MooCodeEventCatch *>(m_stack->get(i)))) {
			// TODO I guess temporarily here we rewind the stack until we think out exception handling more
			for (int j = m_stack->last(); j > i; j--) {
				if ((event = m_stack->pop()))
					delete event;
			}
			m_stack->pop();		/// Pop the handler off the stack but don't delete it yet
			handler->handle(this);
			delete handler;
			return(1);
		}
	}
	return(0);
}


int MooCodeFrame::eval(const char *code, MooArgs *args)
{
	try {
		this->push_code(code);
		return(this->run());
	}
	catch (MooException e) {
		moo_status("EVAL: %s", e.get());
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

