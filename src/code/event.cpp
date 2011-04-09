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
#include <sdm/code/code.h>

typedef int (*MooFormT)(MooCodeFrame *frame, MooCodeExpr *expr);

MooHash<MooFormT *> *form_env = NULL;

MooCodeEvent::MooCodeEvent(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr)
{
	MOO_INCREF(m_env = env);
	MOO_INCREF(m_args = args);
	MOO_INCREF(m_expr = expr);
}

MooCodeEvent::~MooCodeEvent()
{
	MOO_DECREF(m_env);
	MOO_DECREF(m_args);
	MOO_DECREF(m_expr);
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
		frame->set_return(MooObject::resolve(m_expr->get_identifier(), m_env));
		break;
	    }
	    case MCT_CALL: {
		MooFormT *form;
		MooCodeExpr *expr;

		/// If the expr's value is not itself an expr, then the AST is invalid
		if (!(expr = dynamic_cast<MooCodeExpr *>(m_expr->value())))
			throw MooException("(%s, %s) Invalid AST; expected MooCodeExpr.", m_expr->line(), m_expr->col());

		if (expr->expr_type() == MCT_IDENTIFIER) {
			if ((form = form_env->get(expr->get_identifier())))
				return((*form)(frame, expr->next()));
		}
		MooArgs *args = new MooArgs();
		frame->push_event(new MooCodeEventCallExpr(m_env, args));
		frame->push_event(new MooCodeEventEvalArgs(m_env, args, expr));
		MOO_DECREF(args);
		break;
	    }
	    default:
		throw MooException("(%s, %s) Invalid expression type, %d", m_expr->line(), m_expr->col(), m_expr->expr_type());
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

	// TODO the problem here is that you can't pass the frame to evaluate(), so you can't have MooCodeExpr::evaluate()
	//	push the expr in the frame.  It *can* create a new frame, but in this case, it might as well be such that
	//	all MooCode functions have to be actions and they are called through the action interface.  This still does
	//	not deal with the need for MooCodeFunc so we still need evaluate().

	if (!m_args || !(func = m_args->m_args->shift()))
		throw MooException("Null function.");
	func->evaluate(frame, m_env, m_args);
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
	frame->push_event(new MooCodeEventEvalExpr(m_env, NULL, m_expr));
	return(0);
}


/****************************
 * MooCodeEventAppendReturn *
 ****************************/

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

/******************************
 * Form: (set <name> <value>) *
 ******************************/

class FormSetEvent : public MooCodeEvent {
    public:
	FormSetEvent(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		obj = frame->get_return();
		MooObject::resolve(m_expr->get_identifier(), m_env, obj);
		return(0);
	}
};

static int form_set(MooCodeFrame *frame, MooCodeExpr *expr)
{
	// TODO check arguments
	frame->push_event(new FormSetEvent(frame->env(), NULL, expr));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), NULL, expr->next()));
	return(0);
}

/*******************************************************
 * Form: (if <cond:number> <true-expr> [<false-expr>]) *
 *******************************************************/

class FormIfEvent : public MooCodeEvent {
    public:
	FormIfEvent(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		if ((obj = frame->get_return()) && obj->is_true())
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), NULL, m_expr));
		else
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), NULL, m_expr->next()));
		return(0);
	}
};

static int form_if(MooCodeFrame *frame, MooCodeExpr *expr)
{
	// TODO should you check argument numbers?
	//MooCodeExpr *next;
	//if (!(next = expr->next()))

	// TODO wouldn't this cause a segfault?
	frame->push_event(new FormIfEvent(frame->env(), NULL, expr->next()));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), NULL, expr));
	return(0);
}

/****************************
 * Form: (block <expr> ...) *
 ****************************/

static int form_block(MooCodeFrame *frame, MooCodeExpr *expr)
{
	frame->set_return(MOO_INCREF(expr));
	return(0);
}

/**************************************************
 * Form: (lambda ([<identifier>] ...) <expr> ...) *
 **************************************************/

static int form_lambda(MooCodeFrame *frame, MooCodeExpr *expr)
{
	MooCodeLambda *lambda;

	lambda = new MooCodeLambda(expr->get_call(), expr->next());
	frame->set_return(lambda);
	return(0);
}

/***************************************************
 * Form: (foreach <identifier> <array> <expr> ...) *
 ***************************************************/

static int form_foreach(MooCodeFrame *frame, MooCodeExpr *expr)
{
	// TODO you need to make and return a MooCodeLambda class
	//frame->call(global_env->get("#foreach-func", ...)
	return(0);
}


int init_code_event(void)
{
	if (form_env)
		return(1);
	form_env = new MooHash<MooFormT *>(MOO_HBF_REPLACE);

	form_env->set("set", new MooFormT(form_set));
	form_env->set("if", new MooFormT(form_if));
	form_env->set("block", new MooFormT(form_block));
	form_env->set("lambda", new MooFormT(form_lambda));
	form_env->set("foreach", new MooFormT(form_foreach));
	return(0);
}

void release_code_event(void)
{
	if (!form_env)
		return;
	delete form_env;
}


