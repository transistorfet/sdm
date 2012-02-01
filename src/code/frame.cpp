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
#include <sdm/objs/object.h>

#include <sdm/code/code.h>

MooCodeFrame *g_current_frame = NULL;

class FrameEventDebug : public MooCodeEvent {
	std::string m_msg;
    public:
	FrameEventDebug(const char *msg) : MooCodeEvent(NULL, NULL, NULL) {
		m_msg = std::string(msg);
	}

	int do_event(MooCodeFrame *frame) { return(0); }

	void print_debug() {
		moo_status("DEBUG: %s", m_msg.c_str());
	}
};

class FrameEventReturnPoint : public MooCodeEvent {
    public:
	FrameEventReturnPoint() : MooCodeEvent(NULL, NULL, NULL) { };
	int do_event(MooCodeFrame *frame) { return(0); }
};

class FrameEventCatch : public MooCodeEvent {
    public:
	FrameEventCatch(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };

	/// If we evaluate this during the normal run loop then an exception hasn't occurred and we therefore do nothing
	int do_event(MooCodeFrame *frame) { return(0); }
	int handle(MooCodeFrame *frame);
};


MooObjectType moo_code_frame_obj_type = {
	"frame",
	typeid(MooCodeFrame).name(),
	(moo_type_load_t) load_moo_code_frame
};

extern MooObjectHash *global_env;

MooObject *load_moo_code_frame(MooDataFile *data)
{
	MooCodeFrame *obj = new MooCodeFrame();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooCodeFrame::MooCodeFrame(MooObjectHash *env)
{
	m_owner = MooTask::current_owner();
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	m_return = NULL;
	m_exception = NULL;
	m_env = NULL;
	this->env(env ? env : new MooObjectHash());
}

MooCodeFrame::~MooCodeFrame()
{
	MOO_DECREF(m_stack);
	MOO_DECREF(m_env);
	MOO_DECREF(m_return);
	if (m_exception)
		delete m_exception;
}

int MooCodeFrame::clear()
{
	MooCodeEvent *event;

	while ((event = m_stack->pop()))
		delete event;
	return(0);
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


int MooCodeFrame::eval(const char *code, MooObjectArray *args)
{
	try {
		this->push_code(code);
		return(this->run());
	}
	catch (MooException e) {
		moo_status("EVAL: %s", e.get());
		this->print_stacktrace();
		return(-1);
	}
}


int MooCodeFrame::push_event(MooCodeEvent *event)
{
	return(m_stack->push(event));
}

int MooCodeFrame::push_block(MooObjectHash *env, MooCodeExpr *expr)
{
	return(m_stack->push(new MooCodeEventEvalBlock(env, expr)));
}

int MooCodeFrame::push_call(MooObjectHash *env, MooObject *func, MooObjectArray *args)
{
	args->unshift(func);
	return(m_stack->push(new MooCodeEventCallFunc(env, args, func)));
}

int MooCodeFrame::push_method_call(const char *name, MooObject *obj, MooObject *arg1, MooObject *arg2, MooObject *arg3)
{
	MooObject *func;
	MooObjectArray *args;

	if (!(func = obj->resolve_method(name)))
		return(-1);
	args = new MooObjectArray();
	args->set(0, obj);
	if (arg1) {
		args->set(1, arg1);
		if (arg2) {
			args->set(2, arg2);
			if (arg3)
				args->set(3, arg3);
		}
	}

	return(this->push_call(this->env(), func, args));
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
	return(m_stack->push(new FrameEventDebug(buffer)));
}

int MooCodeFrame::run(int limit)
{
	int cycles = 0;
	MooObjectHash *base;
	MooCodeEvent *event;

	if (!limit)
		limit = MOO_FRAME_CYCLE_LIMIT;

	if (g_current_frame)
		throw MooException("Nested run() detected");
	g_current_frame = this;
	base = m_env;
	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	while (cycles <= limit && m_stack->last() >= 0) {
		if (!(event = m_stack->pop()))
			continue;

		if (m_exception) {
			delete m_exception;
			m_exception = NULL;
		}

		try {
			this->env(event->env());
			// TODO temporary for debugging
			//event->print_debug();
			event->do_event(this);
		}
		catch (MooException e) {
			int line, col;

			if (e.is_fatal())
				throw e;
			event->linecol(line, col);
			m_exception = new MooException(e.type(), "(%d, %d): %s", line, col, e.get());
		}
		catch (MooCodeFrameSuspend s) {
			delete event;
			break;
		}

		if (m_exception) {
			m_stack->push(event);
			if (!this->handle_exception()) {
				this->env(base);
				g_current_frame = NULL;
				throw *m_exception;
			}
		}
		else
			delete event;
		cycles++;
	}
	this->env(base);
	g_current_frame = NULL;
	return(cycles);
}

int MooCodeFrame::mark_return_point()
{
	if (dynamic_cast<FrameEventReturnPoint *>(m_stack->get_last()))
		return(0);
	return(m_stack->push(new FrameEventReturnPoint()));
}

int MooCodeFrame::goto_return_point(int level)
{
	int ret = 0;
	MooCodeEvent *event;

	do {
		event = m_stack->pop();
		if (!event)
			return(0);
		if (dynamic_cast<FrameEventReturnPoint *>(event)) {
			level--;
			if (level <= 0)
				ret = 1;
		}
		delete event;
	} while (!ret);
	return(0);
}

int MooCodeFrame::mark_exception(MooCodeExpr *handler)
{
	return(m_stack->push(new FrameEventCatch(m_env, handler)));
}

int MooCodeFrame::handle_exception()
{
	MooCodeEvent *event;
	FrameEventCatch *handler;

	for (int i = m_stack->last(); i >= 0; i--) {
		if ((handler = dynamic_cast<FrameEventCatch *>(m_stack->get(i)))) {
			/// Rewind the stack
			for (int j = m_stack->last(); j > i; j--) {
				if ((event = m_stack->pop()))
					delete event;
			}
			m_stack->pop();		/// Pop the handler off the stack but don't delete it yet
			this->set_return(NULL);
			handler->handle(this);
			delete handler;
			return(1);
		}
	}
	return(0);
}

/*******************
 * FrameEventCatch *
 *******************/

int FrameEventCatch::handle(MooCodeFrame *frame)
{
	if (m_expr)
		frame->push_event(new MooCodeEventEvalBlock(m_env, m_expr));
	return(0);
}

/**********************
 * FrameEventRelegate *
 **********************/

class FrameEventRelegate : public MooCodeEvent {
	moo_id_t m_owner;

    public:
	FrameEventRelegate(moo_id_t owner) : MooCodeEvent(NULL, NULL, NULL) {
		m_owner = owner;
	}

	int do_event(MooCodeFrame *frame) {
		frame->owner(m_owner);
		return(0);
	}
};


int MooCodeFrame::elevate(MooObject *obj)
{
	/// If there is no difference in owner, then don't evelate in the first place.
	//if (obj->owner() == this->owner())
	//	return(0);
	this->push_event(new FrameEventRelegate(m_owner));
	// TODO FIX THIS!!!
	// TODO this is the only time you need a different owner; in this case the owner of the function, which would otherwise be stored
	//	in the hash entry where this object was originially found, but now we don't have it... We could possibly add a MooObject
	//	virtual owner() and over ride in it the few things that have owners???  This still wont help...
//>>	this->owner(obj->owner());
	return(0);
}


void MooCodeFrame::set_return(MooObject *obj)
{
	if (!obj)
		obj = &moo_nil;
	MOO_DECREF(m_return);
	m_return = MOO_INCREF(obj);
}

void MooCodeFrame::env(MooObjectHash *env)
{
	MOO_DECREF(m_env);
	MOO_INCREF(m_env = env);
}

MooObjectHash *MooCodeFrame::extend_env()
{
	m_env = new MooObjectHash(m_env);
	return(m_env);
}

void MooCodeFrame::print_stacktrace()
{
	MooCodeEvent *event;

	for (int i = m_stack->last(); i >= 0; i--) {
		event = m_stack->get(i);
		event->print_debug();
	}
}

