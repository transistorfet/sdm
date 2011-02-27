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

typedef int (*MooFormT)(MooCodeFrame *frame, MooCodeExpr *expr);

MooHash<MooFormT *> *form_env = NULL;

static int code_event_if(MooCodeFrame *frame, MooCodeExpr *expr);

int init_code_event(void)
{
	if (form_env)
		return(1);
	form_env = new MooHash<MooFormT *>(MOO_HBF_REPLACE);

	form_env->set("if", new MooFormT(code_event_if));
	return(0);
}

void release_code_event(void)
{
	if (!form_env)
		return;
	delete form_env;
}

MooCodeEvent::MooCodeEvent(MooObjectHash *env, MooCodeExpr *expr, MooArgs *args)
{
	MOO_INCREF(m_env = env);
	m_expr = expr;
	m_args = args;
}

MooCodeEvent::~MooCodeEvent()
{
	// TODO should you free the m_env reference?
	// TODO this causes a doublefree error since multiple events have a pointer to the same args.  Fix this with libgc or
	//	else add reference management
	//if (m_args)
	//	delete m_args;
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
	    case MCT_OBJECT: {
		frame->set_return(m_expr->value());
		break;
	    }
	    case MCT_IDENTIFIER: {
		MooObject *str, *obj;
		str = m_expr->value();
		if (!(obj = frame->resolve(str->get_string(), m_args)))
			throw MooException("CODE: Undefined reference: %s", str->get_string());
		frame->set_return(obj);
		break;
	    }
	    case MCT_CALL: {
		MooFormT *form;
		MooObject *obj;
		MooCodeExpr *expr, *args_expr;

		/// If the expr's value is not itself an expr, then the AST is invalid
		try {
			expr = dynamic_cast<MooCodeExpr *>(m_expr->value());
		}
		catch (...) {
			throw MooException("CODE: (%s) Invalid AST; expected MooCodeExpr.", m_expr->lineinfo());
		}

		args_expr = expr->next();

		if (expr->expr_type() == MCT_IDENTIFIER) {
			obj = expr->value();
			if ((form = form_env->get(obj->get_string())))
				return((*form)(frame, args_expr));
		}
		MooArgs *args = new MooArgs();
		frame->push_event(new MooCodeEventCallExpr(m_env, args));
		frame->push_event(new MooCodeEventEvalArgs(m_env, expr, args));
		break;
	    }
	    default:
		// TODO you should print line and column number here, perhaps stored in expression during parsing
		throw MooException("CODE: Invalid expression type, %d", m_expr->expr_type());
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


/*************************
 * MooCodeEventCallExpr *
 *************************/

int MooCodeEventCallExpr::do_event(MooCodeFrame *frame)
{
	MooObject *func;
	MooAction *action;
	MooCodeExpr *expr;

	// TODO MooAction::do_action should be changed to MooAction::evaluate()
	// TODO the problem here is that you can't pass the frame to evaluate(), so you can't have MooCodeExpr::evaluate()
	//	push the expr in the frame.  It *can* create a new frame, but in this case, it might as well be such that
	//	all MooCode functions have to be actions and they are called through the action interface.  This still does
	//	not deal with the need for MooCodeFunc so we still need evaluate().

	// TODO MooCodeExpr::evaluate() will create a new frame and thus new environment and thus we don't need teh code below

	// TODO re-evaluate the signifigance of MooAction... (if we are making evaluate())
	if (!m_args || !(func = m_args->m_args->shift()))
		throw MooException("CODE: Null function.");

	/// If the func is not an expr, then the the function is invalid.
	try {
		expr = dynamic_cast<MooCodeExpr *>(func);
	}
	catch (...) {
		try {
			action = dynamic_cast<MooAction *>(func);
		}
		catch (...) {
			// TODO line/col info
			throw MooException("CODE: Invalid function.");
		}
		action->do_action(m_args);
		return(0);
	}

	/// Create a new environment for the function to run in
	MooObjectHash *env = new MooObjectHash();
	// TODO use m_args as the arguments, but how do you pass them? set them in the environment???
	frame->push_event(new MooCodeEventEvalBlock(env, expr));
	return(0);
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


/************************
 * Moo Code Event Forms *
 ************************/

static int code_event_if(MooCodeFrame *frame, MooCodeExpr *expr)
{

	return(0);
}

