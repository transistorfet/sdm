/*
 * Name:	event.cpp
 * Description:	MooCode Event
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>

#include <sdm/objs/thing.h>
#include <sdm/code/code.h>

typedef int (*MooFormT)(MooCodeFrame *frame, MooCodeExpr *expr);

MooHash<MooFormT *> *form_env = NULL;

MooCodeEvent::MooCodeEvent(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr, MooObject *debug)
{
	MOO_INCREF(m_env = env);
	MOO_INCREF(m_args = args);
	MOO_INCREF(m_expr = expr);
	if (debug)
		MOO_INCREF(m_debug = debug);
	else
		MOO_INCREF(m_debug = expr);
}

MooCodeEvent::~MooCodeEvent()
{
	MOO_DECREF(m_env);
	MOO_DECREF(m_args);
	MOO_DECREF(m_expr);
	MOO_DECREF(m_debug);
}

int MooCodeEvent::linecol(int &line, int &col)
{
	MooCodeExpr *expr;

	if (!(expr = dynamic_cast<MooCodeExpr *>(m_debug))) {
		line = 0;
		col = 0;
		return(0);
	}
	line = expr->line();
	col = expr->col();
	return(1);
}

#define DEBUG_LIMIT	77

void MooCodeEvent::print_debug()
{
	int line, col;
	MooCodeExpr *expr;
	char buffer[STRING_SIZE];

	this->linecol(line, col);
	if (m_debug && (expr = dynamic_cast<MooCodeExpr *>(m_debug))) {
		if (MooCodeParser::generate(expr, buffer, DEBUG_LIMIT, &moo_style_one_line) >= DEBUG_LIMIT)
			strcpy(&buffer[DEBUG_LIMIT], "...");
		moo_status("DEBUG: (%d, %d) %s: %s", line, col, typeid(*this).name(), buffer);
	}
	else
		moo_status("DEBUG: (%d, %d) %s", line, col, typeid(*this).name());
}

/************************
 * MooCodeEventEvalExpr *
 ************************/

int MooCodeEventEvalExpr::do_event(MooCodeFrame *frame)
{
	if (!m_expr)
		return(-1);
	switch (m_expr->type()) {
	    case MCT_OBJECT: {
		frame->set_return(m_expr->value());
		break;
	    }
	    case MCT_IDENTIFIER: {
		MooObject *obj;
		const char *name;
		name = m_expr->get_identifier();
		if (!(obj = MooObject::resolve(name, m_env)))
			throw MooException("Undefined value: %s", name);
		frame->set_return(obj);
		break;
	    }
	    case MCT_CALL: {
		MooFormT *form;
		MooCodeExpr *expr;

		/// If the expr's value is not itself an expr, then the AST is invalid
		if (!(expr = dynamic_cast<MooCodeExpr *>(m_expr->value())))
			throw MooException("(%d, %d) Invalid AST; expected MooCodeExpr.", m_expr->line(), m_expr->col());

		if (expr->type() == MCT_IDENTIFIER) {
			if ((form = form_env->get(expr->get_identifier())))
				return((*form)(frame, expr->next()));
		}
		MooArgs *args = new MooArgs();
		// TODO add debug information
		//frame->push_debug("> call to %s");
		frame->push_event(new MooCodeEventCallFunc(m_env, args, m_expr));
		frame->push_event(new MooCodeEventEvalArgs(m_env, args, expr));
		MOO_DECREF(args);
		break;
	    }
	    default:
		throw MooException("(%d, %d) Invalid expression type, %d", m_expr->line(), m_expr->col(), m_expr->type());
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
 * MooCodeEventCallFunc *
 *************************/

int MooCodeEventCallFunc::do_event(MooCodeFrame *frame)
{
	MooObject *func;

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
	// TODO this line causes a segfault for unknown reasons (possibly a double free somewhere??)
	// TODO should this not incref, but instead just steal the ref and set return to NULL? (or otherwise set return to NULL)
	return(m_args->m_args->push(MOO_INCREF(frame->get_return())));
}


/************************
 * Moo Code Event Forms *
 ************************/

/*********************************
 * Form: (define <name> <value>) *
 *********************************/

class FormDefineEvent : public MooCodeEvent {
    public:
	FormDefineEvent(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		obj = frame->get_return();
		MooObject::resolve(m_expr->get_identifier(), m_env, obj);
		return(0);
	}
};

static int form_define(MooCodeFrame *frame, MooCodeExpr *expr)
{
	if (!MooCodeExpr::check_args(expr, 2, 2))
		throw moo_args_mismatched;
	frame->push_event(new FormDefineEvent(frame->env(), expr));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), expr->next()));
	return(0);
}


/******************************
 * Form: (set! <name> <value>) *
 ******************************/

class FormSetEvent : public MooCodeEvent {
    public:
	FormSetEvent(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame) {
		const char *id;
		MooObject *obj;

		obj = frame->get_return();
		id = m_expr->get_identifier();
		if (strchr(id, '.') || strchr(id, ':'))
			throw MooException("Invalid identifier in set");
		m_env->mutate(id, obj);
		return(0);
	}
};

static int form_set(MooCodeFrame *frame, MooCodeExpr *expr)
{
	if (!MooCodeExpr::check_args(expr, 2, 2))
		throw moo_args_mismatched;
	frame->push_event(new FormSetEvent(frame->env(), expr));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), expr->next()));
	return(0);
}

/*******************************************************
 * Form: (if <cond:number> <true-expr> [<false-expr>]) *
 *******************************************************/

class FormIfEvent : public MooCodeEvent {
    public:
	FormIfEvent(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		if ((obj = frame->get_return()) && obj->is_true())
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), m_expr));
		else
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), m_expr->next()));
		return(0);
	}
};

static int form_if(MooCodeFrame *frame, MooCodeExpr *expr)
{
	if (!MooCodeExpr::check_args(expr, 2, 3))
		throw moo_args_mismatched;
	frame->push_event(new FormIfEvent(frame->env(), expr->next()));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), expr));
	return(0);
}

/**************************
 * Form: (and <expr> ...) *
 **************************/

class FormAndEvent : public MooCodeEvent {
    public:
	FormAndEvent(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		if (!(obj = frame->get_return()) || !obj->is_true())
			frame->set_return(new MooBoolean(B_FALSE));
		else if (!m_expr)
			frame->set_return(obj);
		else {
			frame->push_event(new FormAndEvent(frame->env(), m_expr->next()));
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), m_expr));
		}
		return(0);
	}
};

static int form_and(MooCodeFrame *frame, MooCodeExpr *expr)
{
	if (!expr) {
		frame->set_return(new MooBoolean(B_TRUE));
		return(0);
	}
	frame->push_event(new FormAndEvent(frame->env(), expr->next()));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), expr));
	return(0);
}

/**************************
 * Form: (or <expr> ...) *
 **************************/

class FormOrEvent : public MooCodeEvent {
    public:
	FormOrEvent(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame) {
		MooObject *obj;

		if ((obj = frame->get_return()) && obj->is_true())
			frame->set_return(obj);
		else if (!m_expr)
			frame->set_return(obj);
		else {
			frame->push_event(new FormOrEvent(frame->env(), m_expr->next()));
			frame->push_event(new MooCodeEventEvalExpr(frame->env(), m_expr));
		}
		return(0);
	}
};

static int form_or(MooCodeFrame *frame, MooCodeExpr *expr)
{
	if (!expr) {
		frame->set_return(new MooBoolean(B_FALSE));
		return(0);
	}

	frame->push_event(new FormOrEvent(frame->env(), expr->next()));
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), expr));
	return(0);
}

/****************************
 * Form: (begin <expr> ...) *
 ****************************/

static int form_begin(MooCodeFrame *frame, MooCodeExpr *expr)
{
	return(frame->push_block(frame->env(), expr));
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

/*****************************************
 * Form: (super <identifier> <expr> ...) *
 *****************************************/

static int form_super(MooCodeFrame *frame, MooCodeExpr *expr)
{
	MooArgs *args;
	MooObject *othis, *obj;
	MooThing *thing;
	MooMethod *method;
	MooObjectHash *env;

	if (!expr)
		throw moo_args_mismatched;
	env = frame->env();
	if (!(othis = env->get("this")))
		throw MooException("in super: \'this\' not set");
	// TODO  which way should we check for thing? (this is inconsistent with everything else)
	if (!(thing = dynamic_cast<MooThing *>(othis)))
		throw MooException("in super: \'this\' is not a thing");
	if (!(obj = thing->parent()))
		throw MooException("in super: object has no parent");
	obj = obj->resolve_method(expr->get_identifier());
	if ((method = dynamic_cast<MooMethod *>(obj)))
		method->m_obj = othis;
 	args = new MooArgs();
	args->m_args->set(0, MOO_INCREF(obj));
	frame->push_event(new MooCodeEventCallFunc(env, args, expr));
	frame->push_event(new MooCodeEventEvalArgs(env, args, expr->next()));
	MOO_DECREF(args);
	return(0);
}

/*****************************
 * Form: (ignore <expr> ...) *
 *****************************/

static int form_ignore(MooCodeFrame *frame, MooCodeExpr *expr)
{
	frame->mark_exception(NULL);
	frame->push_block(frame->env(), expr);
	return(0);
}

/**************************************************
 * Form: (try <expr> ... (catch (...) <expr> ...) *
 **************************************************/

static int form_try(MooCodeFrame *frame, MooCodeExpr *expr)
{
	const char *str;
	MooCodeExpr *id;
	MooCodeExpr *handler;

	for (handler = expr; handler; handler = handler->next()) {
		if (handler->type() == MCT_CALL && (id = dynamic_cast<MooCodeExpr *>(handler->value()))) {
			str = id->get_string();
			if (str && !strcmp(str, "catch")) {
				handler = id->next();
				break;
			}
		}
	}
	if (!handler)
		throw MooException("No catch expression in try block");
	frame->mark_exception(handler);
	frame->push_block(frame->env(), expr);
	return(0);
}

/****************************
 * Form: (catch <expr> ...) *
 ****************************/

static int form_catch(MooCodeFrame *frame, MooCodeExpr *expr)
{
	/// We are here because we reached the handler part of the try block under normal conditions, therefore we ignore it
	return(0);
}

/***************************
 * Form: (defined? <name>) *
 ***************************/

static int form_defined(MooCodeFrame *frame, MooCodeExpr *expr)
{
	const char *name;

	if (!expr || expr->next())
		throw moo_args_mismatched;

	name = expr->get_identifier();
	if (MooObject::resolve(name, frame->env()))
		frame->set_return(new MooBoolean(B_TRUE));
	else
		frame->set_return(new MooBoolean(B_FALSE));
	return(0);
}

int init_code_event(void)
{
	if (form_env)
		return(1);
	form_env = new MooHash<MooFormT *>(MOO_HBF_REPLACE);

	form_env->set("define", new MooFormT(form_define));
	form_env->set("set!", new MooFormT(form_set));
	form_env->set("if", new MooFormT(form_if));
	form_env->set("and", new MooFormT(form_and));
	form_env->set("or", new MooFormT(form_or));
	form_env->set("begin", new MooFormT(form_begin));
	form_env->set("lambda", new MooFormT(form_lambda));
	form_env->set("super", new MooFormT(form_super));
	form_env->set("ignore", new MooFormT(form_ignore));
	form_env->set("try", new MooFormT(form_try));
	form_env->set("catch", new MooFormT(form_catch));
	form_env->set("defined?", new MooFormT(form_defined));
	return(0);
}

void release_code_event(void)
{
	if (!form_env)
		return;
	delete form_env;
}


