/*
 * Name:	basic.c
 * Description:	Basic Functions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/args.h>
#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>

#include <sdm/code/code.h>

// TODO what will the generic function prototype be?  How will parameters be passed?
// TODO we could do parameters in a lispy way of adding them to the environment by name, and that would all jsut
//	have to be parsed out (the parameter names and that).

/**************************
 * Input/Output Functions *
 **************************/

static int basic_print(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO get_string causes a type error when printing a number, which isn't caught until way up the line (at the nearest
	//	action call, I guess).  This is potentially a very big problem, and doesn't allow for moocode handling of exceptions.
	//args->m_user->notify(TNT_STATUS, args, args->m_args->get_string(0));
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
			args->m_result = new MooInteger((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger((long int) 1);
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
			args->m_result = new MooInteger((long int) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger((long int) 1);
	return(0);
}
/*
static int basic_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
	return(0);
}

static int basic_gt(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i < args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
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
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
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
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
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
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
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

	if (!(obj = args->m_args->get(0)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (moo_data_read_file(buffer, buffer, LARGE_STRING_SIZE) <= 0)
		throw MooException("Unable to read file %s", buffer);
	return(frame->push_code(buffer, args));
}

static int basic_create(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	MooThing *thing, *parent;
	char buffer[LARGE_STRING_SIZE];

	// TODO This should probably be a thing/thingref instead of a string
	if (!(obj = args->m_args->get(0)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	// TODO set parent

	if (!(thing = new MooThing(MOO_NEW_ID, parent->id())))
		throw MooException("Error creating new thing from %d", parent->id());

	if ((obj = args->m_args->get(1))) {
		try {
			//obj->evaluate()
			//(obj:init ...)
		}
		catch (MooException e) {
			// TODO how do you destroy a thing
			//thing->recycle();
			throw e;
		}
	}
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

	env->set("+", new MooCodeFunc(basic_add));
	env->set("-", new MooCodeFunc(basic_subtract));
	env->set("*", new MooCodeFunc(basic_multiply));
	env->set("/", new MooCodeFunc(basic_divide));

	env->set("null", new MooCodeFunc(basic_null));
	env->set("eqv", new MooCodeFunc(basic_eqv));
	//env->set("=", new MooCodeFunc(basic_equal));
	//env->set(">", new MooCodeFunc(basic_gt));
	//env->set(">=", new MooCodeFunc(basic_ge));
	//env->set("<", new MooCodeFunc(basic_lt));
	//env->set("<=", new MooCodeFunc(basic_le));

	env->set("array", new MooCodeFunc(basic_array));
	env->set("hash", new MooCodeFunc(basic_hash));

	env->set("load", new MooCodeFunc(basic_load));
	env->set("@create", new MooCodeFunc(basic_create));
	return(0);
}


