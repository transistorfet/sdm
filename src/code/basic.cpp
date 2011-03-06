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

static int basic_print(MooObjectHash *env, MooArgs *args)
{
	args->m_user->notify(TNT_STATUS, args, args->m_args->get_string(0));
	return(0);
}

static int basic_add(MooObjectHash *env, MooArgs *args)
{
	// TODO what type? double? long int? a different function or something for each?
	for (int i = 0; i < args->m_args->last(); i++) {
		
	}
	return(0);
}

static int basic_subtract(MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int basic_multiply(MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int basic_divide(MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int basic_null(MooObjectHash *env, MooArgs *args)
{
	for (int i = 0; i < args->m_args->last(); i++) {
		if (args->m_args->get(i, NULL)) {
			args->m_result = new MooInteger((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooInteger(1);
	return(0);
}


/*

(set thing:title (lambda ()
	(if (null this.title)
		this.title
		(if (null this.name)
			this.name
			"object")))))

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
	return(0);
}


