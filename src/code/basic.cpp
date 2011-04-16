/*
 * Name:	basic.c
 * Description:	Basic Functions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>


/**************************
 * Input/Output Functions *
 **************************/

static int basic_print(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	// TODO this is temporary until you get notify refactored
	args->m_channel = dynamic_cast<MooThing *>(env->get("channel"));
	if (!(obj = args->m_args->get(0)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	args->m_user->notify(TNT_STATUS, args, buffer);
	return(0);
}

static int basic_format(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *str;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	str = args->m_args->get_string(0);
	MooObject::format(buffer, LARGE_STRING_SIZE, env, str);
	args->m_result = new MooString(buffer);
	return(0);
}

/******************
 * Math Functions *
 ******************/

static int basic_add(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int i = 0;
	double d = 0;
	MooObject *obj;

	// TODO what type? double? long int? a different function or something for each?
	for (int i = 0; i < args->m_args->last(); i++) {
		//obj = args->m_args->get(i);
		// TODO wtf, seriously
	}
	return(0);
}

static int basic_subtract(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int basic_multiply(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int basic_divide(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{

	return(0);
}

/************************
 * Comparison Functions *
 ************************/

static int basic_null(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	for (int i = 0; i < args->m_args->last(); i++) {
		if (args->m_args->get(i)) {
			args->m_result = new MooNumber((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber((long int) 1);
	return(0);
}

static int basic_eqv(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) != obj) {
			args->m_result = new MooNumber((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber((long int) 1);
	return(0);
}

static int basic_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("Expected number at arg 0");
	for (int i = 1; i < args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("Expected number at arg %d", i);
		if (!num1->equals(num2)) {
			args->m_result = new MooNumber((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber((long int) 1);
	return(0);
}

static int basic_not_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("Expected number at arg 0");
	for (int i = 1; i < args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("Expected number at arg %d", i);
		if (num1->equals(num2)) {
			args->m_result = new MooNumber((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber((long int) 1);
	return(0);
}

/*
static int basic_gt(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooNumber((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber(1);
	return(0);
}

static int basic_ge(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooNumber((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber(1);
	return(0);
}

static int basic_lt(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooNumber((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber(1);
	return(0);
}

static int basic_le(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooNumber((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber(1);
	return(0);
}
*/

/******************
 * Type Functions *
 ******************/

static int basic_array(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	args->m_result = args->m_args;
	args->m_args = NULL;
	return(0);
}

static int basic_hash(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO for now, we wont initialize the hash
	if (args->m_args->last() >= 0)
		throw moo_args_mismatched;
	args->m_result = new MooObjectHash();
	return(0);
}

/********************
 * System Functions *
 ********************/

static int basic_load(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (moo_data_read_file(buffer, buffer, LARGE_STRING_SIZE) <= 0)
		throw MooException("Unable to read file %s", buffer);
	return(frame->push_code(buffer, args));
}

static int basic_clone(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	MooThing *thing, *parent;

	if (args->m_args->last() < 0 || args->m_args->last() > 1)
		throw moo_args_mismatched;
	if (!(parent = args->m_args->get_thing(0)))
		return(-1);

	if (!(thing = new MooThing(MOO_NEW_ID, parent->id())))
		throw MooException("Error creating new thing from %d", parent->id());
	// TODO clone all properties from parent
	// TODO call thing:init ??

	if ((obj = args->m_args->get(1)))
		frame->push_call(env, obj, new MooArgs());
	args->m_result = thing;
	return(0);
}


/*

(set thing:title (lambda ()
	(if (null this.title)
		this.title
		(if (null this.name)
			this.name
			"object")))))


; This is an action with a helper function inside of it.  This should be made to work somehow so that complex actions can use
; localized helper functions without poluting other namespaces.  Also, moocode actions should be able to parse the args into
; variables in the env based an a param list, just like lambda does.
; TODO we still need to figure out how command lines should be parsed into moocode actions...  Where will the parsing take place
;	and based on what information (like from <params></params>)
(action (x)
	(set fac (lambda (n)
		(if (= n 0)
			1
			(* n (fac (- n 1)))))
	(print "The factorial of " x " is " (fac x)))

*/

int moo_load_code_basic(MooObjectHash *env)
{
	// TODO this should be an actual callable function, something that the core executor can use to call the function
	// TODO right now, it's just a MooCodeExpr, but we should maybe add parameters? or actually... we could have the
	//	first expression to be a (lambda (x y) ...) type function (ie. it calls a primative, "lambda", which takes
	//	a specially parsed name list, creates a new environment, adds values (where does it get them from???) to
	//	the environment by name, and then pushes a new event on the stack to evaluate a sub-expr using the created
	//	environment (and then the env relies on garbage collection to be freed)

	env->set("print", new MooCodeFunc(basic_print));
	env->set("format", new MooCodeFunc(basic_format));

	env->set("+", new MooCodeFunc(basic_add, "&all"));
	env->set("-", new MooCodeFunc(basic_subtract, "&all"));
	env->set("*", new MooCodeFunc(basic_multiply, "&all"));
	env->set("/", new MooCodeFunc(basic_divide, "&all"));

	env->set("null", new MooCodeFunc(basic_null, "&all"));
	env->set("eqv", new MooCodeFunc(basic_eqv, "&all"));
	env->set("=", new MooCodeFunc(basic_equal, "&all"));
	env->set("!=", new MooCodeFunc(basic_not_equal, "&all"));
	//env->set(">", new MooCodeFunc(basic_gt));
	//env->set(">=", new MooCodeFunc(basic_ge));
	//env->set("<", new MooCodeFunc(basic_lt));
	//env->set("<=", new MooCodeFunc(basic_le));

	env->set("array", new MooCodeFunc(basic_array));
	env->set("hash", new MooCodeFunc(basic_hash));

	env->set("load", new MooCodeFunc(basic_load));
	env->set("@clone", new MooCodeFunc(basic_clone));
	return(0);
}


