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
static int code_event_block(MooCodeFrame *frame, MooCodeExpr *expr);
static int code_event_lambda(MooCodeFrame *frame, MooCodeExpr *expr);

int init_code_event(void)
{
	if (form_env)
		return(1);
	form_env = new MooHash<MooFormT *>(MOO_HBF_REPLACE);

	form_env->set("if", new MooFormT(code_event_if));
	form_env->set("block", new MooFormT(code_event_block));
	form_env->set("lambda", new MooFormT(code_event_lambda));
	return(0);
}

void release_code_event(void)
{
	if (!form_env)
		return;
	delete form_env;
}

MooCodeEvent::MooCodeEvent(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr)
{
	MOO_INCREF(m_env = env);
	MOO_INCREF(m_args = args);
	m_expr = expr;
}

MooCodeEvent::~MooCodeEvent()
{
	MOO_DECREF(m_env);
	MOO_DECREF(m_args);
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
			throw MooException("Undefined reference: %s", str->get_string());
		frame->set_return(obj);
		break;
	    }
	    case MCT_CALL: {
		MooFormT *form;
		MooObject *obj;
		MooCodeExpr *expr;

		/// If the expr's value is not itself an expr, then the AST is invalid
		if (!(expr = dynamic_cast<MooCodeExpr *>(m_expr->value())))
			throw MooException("(%s) Invalid AST; expected MooCodeExpr.", m_expr->lineinfo());

		if (expr->expr_type() == MCT_IDENTIFIER) {
			obj = expr->value();
			if ((form = form_env->get(obj->get_string())))
				return((*form)(frame, expr->next()));
		}
		// TODO initialize with previous call's arguments (where get?) (either this->m_args or frame->m_args)
		MooArgs *args = new MooArgs(m_args);
		frame->push_event(new MooCodeEventCallExpr(m_env, args));
		frame->push_event(new MooCodeEventEvalArgs(m_env, args, expr));
		MOO_DECREF(args);
		break;
	    }
	    default:
		// TODO you should print line and column number here, perhaps stored in expression during parsing
		throw MooException("Invalid expression type, %d", m_expr->expr_type());
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
		frame->push_event(new MooCodeEventEvalBlock(m_env, m_args, remain));
	frame->push_event(new MooCodeEventEvalExpr(m_env, m_args, m_expr));
	return(0);
}


/*************************
 * MooCodeEventCallExpr *
 *************************/

int MooCodeEventCallExpr::do_event(MooCodeFrame *frame)
{
	MooObject *func;
	MooAction *action;

	// TODO MooAction::do_action should be changed to MooAction::evaluate()
	// TODO the problem here is that you can't pass the frame to evaluate(), so you can't have MooCodeExpr::evaluate()
	//	push the expr in the frame.  It *can* create a new frame, but in this case, it might as well be such that
	//	all MooCode functions have to be actions and they are called through the action interface.  This still does
	//	not deal with the need for MooCodeFunc so we still need evaluate().

	// TODO MooCodeExpr::evaluate() will create a new frame and thus new environment and thus we don't need teh code below

	// TODO re-evaluate the signifigance of MooAction... (if we are making evaluate())
	if (!m_args || !(func = m_args->m_args->shift()))
		throw MooException("Null function.");

/*
	/// If the func is not an expr, then the the function is invalid.
	if ((expr = dynamic_cast<MooCodeExpr *>(func))) {
		/// Create a new environment for the function to run in
		MooObjectHash *env = new MooObjectHash();
		// TODO use m_args as the arguments, but how do you pass them? set them in the environment???
		frame->push_event(new MooCodeEventEvalBlock(env, expr));
		m_args->m_result = frame.get_return();
	}
*/
	if ((action = dynamic_cast<MooAction *>(func)))
		action->do_action(m_args);
	else
		func->evaluate(m_env, m_args);
	frame->set_return(m_args->m_result);
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
		frame->push_event(new MooCodeEventEvalArgs(m_env, m_args, remain));
	frame->push_event(new MooCodeEventAppendReturn(m_env, m_args));
	frame->push_event(new MooCodeEventEvalExpr(m_env, m_args->m_parent, m_expr));
	return(0);
}


/****************************
 * MooCodeEventAppendReturn *
 ****************************/

#include <sdm/objs/integer.h>
int MooCodeEventAppendReturn::do_event(MooCodeFrame *frame)
{
	if (!m_args)
		return(-1);
	// TODO this line causes a segfault for unknown reasons (possibly a double free somewhere??)
	// TODO should this not incref, but instead just steal the ref and set return to NULL? (or otherwise set return to NULL)
	return(m_args->m_args->push(MOO_INCREF(frame->get_return())));
}


/************************
 * Moo Code Event Forms *
 ************************/

static int code_event_if(MooCodeFrame *frame, MooCodeExpr *expr)
{

	return(0);
}

static int code_event_block(MooCodeFrame *frame, MooCodeExpr *expr)
{
	// TODO do you need to incref expr?
	frame->set_return(expr);
	return(0);
}

static int code_event_lambda(MooCodeFrame *frame, MooCodeExpr *expr)
{
	// TODO you need to make and return a MooCodeLambda class
	return(0);
}

