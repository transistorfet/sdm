/*
 * Header Name:	event.h
 * Description:	MooCode Event
 */

#ifndef _SDM_CODE_EVENT_H
#define _SDM_CODE_EVENT_H

#include <sdm/globals.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>
#include <sdm/code/parser.h>

class MooCodeEvent;
class MooCodeFrame;

int init_code_event(void);
void release_code_event(void);

class MooCodeEvent {
    protected:
	MooCodeExpr *m_debug;
	MooObjectHash *m_env;
	MooCodeExpr *m_expr;
	MooObjectArray *m_args;

    public:
	MooCodeEvent(MooCodeExpr *debug, MooObjectHash *env, MooObjectArray *args = NULL, MooCodeExpr *expr = NULL);
	virtual ~MooCodeEvent();
	MooObjectHash *env() { return(m_env); }

	virtual int do_event(MooCodeFrame *frame) = 0;

	virtual void print_debug();
	int linecol(int &line, int &col);
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(expr, env, NULL, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalBlock : public MooCodeEvent {
    public:
	MooCodeEventEvalBlock(MooCodeExpr *debug, MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(debug, env, NULL, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventCallFunc : public MooCodeEvent {
    public:
	MooCodeEventCallFunc(MooCodeExpr *debug, MooObjectHash *env, MooObjectArray *args) : MooCodeEvent(debug, env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalArgs : public MooCodeEvent {
    public:
	MooCodeEventEvalArgs(MooCodeExpr *debug, MooObjectHash *env, MooObjectArray *args, MooCodeExpr *expr) : MooCodeEvent(debug, env, args, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooCodeExpr *debug, MooObjectHash *env, MooObjectArray *args) : MooCodeEvent(debug, env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventReturnPoint : public MooCodeEvent {
    public:
	MooCodeEventReturnPoint() : MooCodeEvent(NULL, NULL, NULL) { };
	int do_event(MooCodeFrame *frame) { return(0); }
};

class MooCodeEventCatch : public MooCodeEvent {
    public:
	MooCodeEventCatch(MooCodeExpr *debug, MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(debug, env, NULL, expr) { };
	/// If we evaluate this during the normal run loop then an exception hasn't occurred and we therefore do nothing
	int do_event(MooCodeFrame *frame) { return(0); }
	int handle(MooCodeFrame *frame, MooException *e);
};

class MooCodeEventDebug : public MooCodeEvent {
	std::string m_msg;
    public:
	MooCodeEventDebug(const char *msg) : MooCodeEvent(NULL, NULL, NULL) {
		m_msg = std::string(msg);
	}

	int do_event(MooCodeFrame *frame) { return(0); }

	void print_debug() {
		moo_status("DEBUG: %s", m_msg.c_str());
	}
};

#endif

