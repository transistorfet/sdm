/*
 * Header Name:	event.h
 * Description:	MooCode Event
 */

#ifndef _SDM_ACTIONS_CODE_EVENT_H
#define _SDM_ACTIONS_CODE_EVENT_H

#include <sdm/array.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

#include "expr.h"

class MooCodeEvent;
class MooCodeFrame;

class MooCodeEvent {
    public:
	MooObjectHash *m_env;
	MooCodeExpr *m_expr;
	MooArgs *m_args;

	MooCodeEvent(MooObjectHash *env, MooCodeExpr *expr = NULL, MooArgs *args = NULL);
	virtual ~MooCodeEvent();
	void set_args(MooArgs *args);

	virtual int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalBlock : public MooCodeEvent {
    public:
	MooCodeEventEvalBlock(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, expr) { };
	int do_event(MooCodeFrame *frame);
};


class MooCodeEventDoAction : public MooCodeEvent {
	MooThing *m_thing;
	std::string *m_action;
    public:
	MooCodeEventDoAction(MooObjectHash *env, MooThing *thing, const char *action, MooArgs *args);
	virtual ~MooCodeEventDoAction();
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalArgs : public MooCodeEvent {
    public:
	MooCodeEventEvalArgs(MooObjectHash *env, MooCodeExpr *expr, MooArgs *args) : MooCodeEvent(env, expr, args) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooObjectHash *env, MooArgs *args) : MooCodeEvent(env, NULL, args) { };
	int do_event(MooCodeFrame *frame);
};

#endif

