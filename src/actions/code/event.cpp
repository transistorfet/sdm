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
#include "frame.h"

MooCodeEvent::MooCodeEvent(MooObjectHash *env, MooCodeExpr *expr, MooArgs *args)
{
	m_env = env;		// TODO do you make a reference?
	m_expr = expr;
	m_args = args;
}

MooCodeEvent::~MooCodeEvent()
{
	// TODO should you free the m_env reference?
	if (m_args)
		delete m_args;
}

void MooCodeEvent::set_args(MooArgs *args)
{
	// TODO should this instead make the args?
	if (m_args)
		delete m_args;
	m_args = args;
}

int MooCodeEvent::do_event(MooCodeFrame *frame)
{
	return(-1);
}


/************************
 * MooCodeEventEvalExpr *
 ************************/

int MooCodeEventEvalExpr::do_event(MooCodeFrame *frame)
{
	if (!m_expr)
		return(-1);
	switch (m_expr->expr_type()) {
	    case MCT_OBJECT:
		// TODO should you make a reference to m_value or something?  Should set_return?
		frame->set_return(m_expr->value());
		break;
	    case MCT_IDENTIFIER:
		// TODO do variable lookup, use frame to find the evironment/table to look up
		//	do a lookup on the frame env, then a lookup on global_env, then give up (since we don't have nested envs)
		break;
	    case MCT_CALL:
		MooCodeExpr *id_expr, *args_expr;

		/// If the expr's value is not itself an expr, then the AST is invalid
		try {
			id_expr = dynamic_cast<MooCodeExpr *>(m_expr->value());
		}
		catch (...) {
			throw MooException("CODE: (%s) Invalid AST; expected MooCodeExpr.", m_expr->lineinfo());
		}

		args_expr = id_expr->next();
		/*
			push CallFunction(  **something**  )
			push EvalArgs(args_expr, args)
			push DoFunction(args_expr)	// frame->return() is the function value to execute
			push EvalExpr(id_expr)

		*/


		// TODO break m_value into first expr and remain expr...
		// TODO if first expr is an identifier, then do a lookup
		// TODO if first expr is an object, and it's an expr, then use this as body
		// TODO if first expr is a call, then somehow do a recursive call thing

		// TODO should m_args have a reference created? or sholud the constructor create a ref perhaps??
/*
		if (***function is a code/primative function***) {
			frame->push_event(new MooCodeEvalBlock(***function expr body***, ***remain... but as MooArgs??***));
		}
		else {
			MooArgs *args = new MooArgs();
			frame->push_event(new MooCodeEventEvalBlock(***function expr body***, args));
			frame->push_event(new MooCodeEventEvalArgs(m_env, remain, args));
		}
*/
		break;
	    default:
		return(-1);
	}
	return(0);
}


/*************************
 * MooCodeEventEvalBlock *
 *************************/

int MooCodeEventEvalBlock::do_event(MooCodeFrame *frame)
{
	MooCodeExpr *remain;

	if (!m_expr)
		return(-1);
	remain = m_expr->next();

	if (remain)
		frame->push_event(new MooCodeEventEvalBlock(m_env, remain));
	frame->push_event(new MooCodeEventEvalExpr(m_env, m_expr));
	return(0);
}


/************************
 * MooCodeEventDoAction *
 ************************/

MooCodeEventDoAction::MooCodeEventDoAction(MooObjectHash *env, MooThing *thing, const char *action, MooArgs *args) : MooCodeEvent(env, NULL, args)
{
	m_thing = thing;
	m_action = new std::string(action);
}

MooCodeEventDoAction::~MooCodeEventDoAction()
{
	if (m_action)
		delete m_action;
}

int MooCodeEventDoAction::do_event(MooCodeFrame *frame)
{
	int res;
	MooObject *result;

	if (!m_args || !m_thing || !m_action)
		return(-1);
	res = m_thing->do_action(m_action->c_str(), m_args, &result);
	if (res < 0)
		return(res);
	frame->set_return(result);
	return(res);
}


/************************
 * MooCodeEventEvalArgs *
 ************************/

int MooCodeEventEvalArgs::do_event(MooCodeFrame *frame)
{
	MooCodeExpr *remain;

	if (!m_expr)
		return(-1);
	remain = m_expr->next();

	if (remain)
		frame->push_event(new MooCodeEventEvalArgs(m_env, remain, m_args));
	frame->push_event(new MooCodeEventAppendReturn(m_env, m_args));
	frame->push_event(new MooCodeEventEvalExpr(m_env, m_expr));
	return(0);
}


/****************************
 * MooCodeEventAppendReturn *
 ****************************/

int MooCodeEventAppendReturn::do_event(MooCodeFrame *frame)
{
	if (!m_args)
		return(-1);
	// TODO do you need to inc a ref here on the return value?  Should it be embedded in one of these functions?
	return(m_args->m_args->push(frame->get_return()));
}

