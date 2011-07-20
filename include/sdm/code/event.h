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
	MooObjectHash *m_env;
	MooCodeExpr *m_expr;
	MooArgs *m_args;
	MooObject *m_debug;

    public:
	MooCodeEvent(MooObjectHash *env, MooArgs *args = NULL, MooCodeExpr *expr = NULL, MooObject *debug = NULL);
	virtual ~MooCodeEvent();
	MooObjectHash *env() { return(m_env); }

	virtual int do_event(MooCodeFrame *frame) = 0;

	virtual void print_debug();
	int linecol(int &line, int &col);
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooObjectHash *env, MooCodeExpr *expr, MooObject *debug = NULL) : MooCodeEvent(env, NULL, expr, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalBlock : public MooCodeEvent {
    public:
	MooCodeEventEvalBlock(MooObjectHash *env, MooCodeExpr *expr, MooObject *debug = NULL) : MooCodeEvent(env, NULL, expr, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventCallFunc : public MooCodeEvent {
    public:
	MooCodeEventCallFunc(MooObjectHash *env, MooArgs *args, MooObject *debug = NULL) : MooCodeEvent(env, args, NULL, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalArgs : public MooCodeEvent {
    public:
	MooCodeEventEvalArgs(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr, MooObject *debug = NULL) : MooCodeEvent(env, args, expr, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooObjectHash *env, MooArgs *args, MooObject *debug = NULL) : MooCodeEvent(env, args, NULL, NULL) { };
	int do_event(MooCodeFrame *frame);
};

#endif

