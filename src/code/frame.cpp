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

//#define TASK_LIST_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE

extern MooObjectHash *global_env;

MooCodeFrame *g_current_frame = NULL;
//static MooArray<MooCodeFrame *> *g_task_list = NULL;
static MooTaskQueue *g_task_queue = NULL;

int init_frame(void)
{
	if (g_task_queue)
		return(1);
//	g_task_list = new MooArray<MooCodeFrame *>(MOO_ARRAY_DEFAULT_SIZE, -1, TASK_LIST_BITS);
	g_task_queue = new MooTaskQueue();
	return(0);
}

void release_frame(void)
{
	delete g_task_queue;
//	delete g_task_list;
}


/***************************
 * MooCodeFrame Definition *
 ***************************/

MooCodeFrame::MooCodeFrame(MooObjectHash *env)
{
	m_owner = g_current_frame ? g_current_frame->m_owner : -1;
	m_stack = new MooArray<MooCodeEvent *>(5, -1, MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE);
	m_curevent = NULL;
	m_return = NULL;
	m_exception = NULL;
	m_env = NULL;
	this->env(env ? env : new MooObjectHash());
}

MooCodeFrame::~MooCodeFrame()
{
	if (this == g_current_frame)
		g_current_frame = NULL;
	MOO_DECREF(m_stack);
	MOO_DECREF(m_env);
	MOO_DECREF(m_return);
	if (m_curevent)
		delete m_curevent;
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
	return(m_stack->push(new MooCodeEventEvalBlock(expr, env, expr)));
}

int MooCodeFrame::push_call(MooObjectHash *env, MooObjectArray *args)
{
	return(m_stack->push(new MooCodeEventCallFunc(NULL, env, args)));
}

int MooCodeFrame::push_call(MooObjectHash *env, MooObject *func, MooObjectArray *args)
{
	args->unshift(func);
	return(m_stack->push(new MooCodeEventCallFunc(NULL, env, args)));
}

int MooCodeFrame::push_method_call(const char *name, MooObject *obj, MooObject *arg1, MooObject *arg2, MooObject *arg3)
{
	MooObject *func;
	MooObjectArray *args;

	if (!(func = obj->resolve_method(name)))
		return(-1);
	args = new MooObjectArray();
	args->set(0, func);
	args->set(1, obj);
	if (arg1) {
		args->set(2, arg1);
		if (arg2) {
			args->set(3, arg2);
			if (arg3)
				args->set(4, arg3);
		}
	}

	return(this->push_call(this->env(), args));
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

int MooCodeFrame::run_all()
{
	int ret = 0;
	MooCodeFrame *frame;

	while ((frame = g_task_queue->consume())) {
		if (frame->run())
			ret = 1;
	}
	return(ret);
}

int MooCodeFrame::schedule(double time)
{
	// TODO remove the task before rescheduling it
	g_task_queue->schedule(this, time);
	if (g_current_frame == this)
		throw MooCodeFrameSuspend();
	return(0);
}

int MooCodeFrame::run()
{
	int cycles;
	clock_t start;

	start = clock();
	try {
		cycles = this->do_run();
	}
	catch (MooException e) {
		// TODO temporary for debugging purposes??
		moo_status("EXEC: %s", e.get());
		this->print_stacktrace();
		cycles = -1;

		//this->clear();

		// TODO you need some way of reporting the error back to the user
		// TODO the problem is you need to call basic_print, but it requires a frame and it calls a method, so you need to
		//	either run the frame back to it's starting point (I suppose you could even push a breakpoint or something on
		//	to the event stack rather than make custom provisions in frame->run().)  You need to detect double errors though
		//	where an error occurs in the new code being called so you don't loop infinitely.  You could also somehow package
		//	the event stack into an exception, clear everything, and then push the error reporting code.
		/*
		channel = dynamic_cast<MooThing *>(env->get("channel"));
		if ((thing = MooThing::lookup(MooTask::current_user())))
			thing->notify(TNT_STATUS, NULL, channel, e.get());
		*/
	}
	//moo_status("Executed (%s ...) in %f seconds", name, ((float) clock() - start) / CLOCKS_PER_SEC);
	return(cycles);
}

int MooCodeFrame::do_run(int limit)
{
	int cycles = 0;
	MooObjectHash *base;

	if (!limit)
		limit = MOO_FRAME_CYCLE_LIMIT;

	if (g_current_frame)
		throw MooException("Nested run() detected");
	g_current_frame = this;

	if (m_exception) {
		delete m_exception;
		m_exception = NULL;
	}

	if (m_curevent) {
		delete m_curevent;
		m_curevent = NULL;
	}

	base = m_env;
	// TODO add an event counter in the frame and also take a max events param or something, such that
	//	a frame gets a limited time slice...
	while (cycles <= limit && m_stack->last() >= 0) {
		if (m_exception) {
			delete m_exception;
			m_exception = NULL;
		}

		if (!(m_curevent = m_stack->pop()))
			continue;

		try {
			this->env(m_curevent->env());
			// TODO temporary for debugging
			//event->print_debug();
			m_curevent->do_event(this);
		}
		catch (MooException e) {
			int line, col;

			if (e.is_fatal())
				throw e;
			m_curevent->linecol(line, col);
			m_exception = new MooException(e.type(), "(%d, %d): %s", line, col, e.get());
		}
		catch (MooCodeFrameSuspend s) {
			delete m_curevent;
			m_curevent = NULL;
			break;
		}

		if (m_exception) {
			if (!this->handle_exception())
				break;
			delete m_exception;
			m_exception = NULL;
		}

		delete m_curevent;
		m_curevent = NULL;
		cycles++;
	}

	if (m_curevent) {
		delete m_curevent;
		m_curevent = NULL;
	}

	this->env(base);
	g_current_frame = NULL;
	if (m_exception)
		throw *m_exception;
	return(cycles);
}

int MooCodeFrame::mark_return_point()
{
	if (dynamic_cast<MooCodeEventReturnPoint *>(m_stack->get_last()))
		return(0);
	return(m_stack->push(new MooCodeEventReturnPoint()));
}

int MooCodeFrame::goto_return_point(int level)
{
	int ret = 0;
	MooCodeEvent *event;

	do {
		event = m_stack->pop();
		if (!event)
			return(0);
		if (dynamic_cast<MooCodeEventReturnPoint *>(event)) {
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
	// TODO add debug field somehow
	return(m_stack->push(new MooCodeEventCatch(NULL, m_env, handler)));
}

int MooCodeFrame::handle_exception()
{
	MooCodeEvent *event;
	MooCodeEventCatch *handler;

	for (int i = m_stack->last(); i >= 0; i--) {
		if ((handler = dynamic_cast<MooCodeEventCatch *>(m_stack->get(i)))) {
			/// Rewind the stack
			for (int j = m_stack->last(); j > i; j--) {
				if ((event = m_stack->pop()))
					delete event;
			}
			m_stack->pop();		/// Pop the handler off the stack but don't delete it yet
			this->set_return(NULL);
			handler->handle(this, m_exception);
			delete handler;
			return(1);
		}
	}
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
	//this->push_event(new FrameEventRelegate(m_owner));
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

MooObject *MooCodeFrame::resolve(const char *name, MooObject *value, MooObject **parent)
{
	MooObject *obj;
	char *method, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	if ((method = strchr(buffer, ':'))) {
		*method = '\0';
		method++;
	}

	if ((remain = strchr(buffer, '.'))) {
		*remain = '\0';
		remain++;
	}

	if (!method && !remain && value) {
		// TODO check perms
		if (!m_env) {
			if (global_env->set(buffer, MOO_INCREF(value)))
				return(NULL);
		}
		else if (m_env->set(buffer, MOO_INCREF(value)))
			return(NULL);
		return(value);
	}
	else {
		// TODO should we modify this so that we never do a global_env lookup and instead rely on the env being linked to global_env
		if (!(obj = MooMutable::reference(buffer))
		    && !(obj = m_env->get(buffer))
		    && !(obj = global_env->get(buffer)))
			return(NULL);
	}
	if (remain && !(obj = obj->resolve_property(remain, method ? NULL : value)))
		return(NULL);

	if (method) {
		if (parent)
			*parent = obj;
		if (!(obj = obj->resolve_method(method, value)))
			return(NULL);
	}
	return(obj);
}


void MooCodeFrame::print_stacktrace()
{
	MooCodeEvent *event;

	for (int i = m_stack->last(); i >= 0; i--) {
		event = m_stack->get(i);
		event->print_debug();
	}
}

