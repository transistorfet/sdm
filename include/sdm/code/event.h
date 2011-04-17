/*
 * Header Name:	event.h
 * Description:	MooCode Event
 */

#ifndef _SDM_CODE_EVENT_H
#define _SDM_CODE_EVENT_H

#include <sdm/array.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>

class MooCodeEvent;
class MooCodeFrame;

int init_code_event(void);
void release_code_event(void);

class MooCodeEvent {
    protected:
	MooObjectHash *m_env;
	MooCodeExpr *m_expr;
	MooArgs *m_args;

    public:
	MooCodeEvent(MooObjectHash *env, MooArgs *args = NULL, MooCodeExpr *expr = NULL);
	virtual ~MooCodeEvent();
	MooObjectHash *env() { return(m_env); }
	MooCodeExpr *expr() { return(m_expr); }

	virtual int do_event(MooCodeFrame *frame);

	inline int linecol(int &line, int &col) {
		if (!m_expr)
			return(0);
		line = m_expr->line();
		col = m_expr->col();
		return(1);
	}
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalBlock : public MooCodeEvent {
    public:
	MooCodeEventEvalBlock(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventCallExpr : public MooCodeEvent {
    public:
	MooCodeEventCallExpr(MooObjectHash *env, MooArgs *args) : MooCodeEvent(env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalArgs : public MooCodeEvent {
    public:
	MooCodeEventEvalArgs(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooObjectHash *env, MooArgs *args) : MooCodeEvent(env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

#endif

