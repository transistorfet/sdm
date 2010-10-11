/*
 * Name:	event.cpp
 * Description:	MooCode Event
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
#include <sdm/actions/action.h>
#include <sdm/actions/code/code.h>

#include "expr.h"
#include "event.h"
#include "thread.h"

MooCodeEvent::MooCodeEvent(MooCodeExpr *expr, MooArgs *args)
{
	m_expr = expr;
	m_args = args;
}

MooCodeEvent::MooCodeEvent(moo_event_func_t func, MooCodeExpr *expr, MooArgs *args)
{
	m_func = func;
	m_expr = expr;
	m_args = args;
}

MooCodeEvent::~MooCodeEvent()
{
	// TODO destroy references??
}

void MooCodeEvent::set_args(MooArgs *args)
{
	// TODO should this instead make the args?
	if (m_args)
		delete m_args;
	m_args = args;
}

int MooCodeEvent::do_event(MooCodeThread *thread)
{
	if (!m_func)
		return(-1);
	//return((this)->*(m_func)(thread));
}

int MooCodeEventEvalExpr::do_event(MooCodeThread *thread)
{
	if (!m_expr)
		return(-1);
	return(m_expr->eval(thread));
}

int MooCodeEventEvalExprList::do_event(MooCodeThread *thread)
{
	MooCodeExpr *remain;
	MooCodeEvent *event;

	if (!m_expr)
		return(-1);
	remain = m_expr->next();

	if (remain) {
		event = new MooCodeEventEvalExprList(remain);
		thread->push_event(event);
	}

	event = new MooCodeEventAppendReturn(NULL, m_args);
	thread->push_event(event);

	event = new MooCodeEventEvalExpr(m_expr);
	thread->push_event(event);
	return(0);
}

int MooCodeEventAppendReturn::do_event(MooCodeThread *thread)
{
	if (!m_args)
		return(-1);
	return(m_args->m_args->push(thread->get_return()));
}

