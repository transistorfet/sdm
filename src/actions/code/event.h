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
class MooCodeThread;

typedef int (MooCodeEvent::*moo_event_func_t)(MooCodeThread *thread);

class MooCodeEvent {
    public:
	moo_event_func_t m_func;
	MooCodeExpr *m_expr;
	MooArgs *m_args;

	MooCodeEvent(MooCodeExpr *expr, MooArgs *args = NULL);
	MooCodeEvent(moo_event_func_t func = NULL, MooCodeExpr *expr = NULL, MooArgs *args = NULL);
	virtual ~MooCodeEvent();
	void set_args(MooArgs *args);

	virtual int do_event(MooCodeThread *thread);

	int evaluate_expr(MooCodeThread *thread);
	int evaluate_expr_list(MooCodeThread *thread);
	int append_return(MooCodeThread *thread);
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooCodeExpr *expr, MooArgs *args = NULL) : MooCodeEvent(expr, args) { };
	int do_event(MooCodeThread *thread);
};

class MooCodeEventEvalExprList : public MooCodeEvent {
    public:
	MooCodeEventEvalExprList(MooCodeExpr *expr, MooArgs *args = NULL) : MooCodeEvent(expr, args) { };
	int do_event(MooCodeThread *thread);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooCodeExpr *expr, MooArgs *args = NULL) : MooCodeEvent(expr, args) { };
	int do_event(MooCodeThread *thread);
};

#endif

