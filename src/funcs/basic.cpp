/*
 * Name:	basic.cpp
 * Description:	Basic Functions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/things/user.h>
#include <sdm/things/thing.h>


/**************************
 * Input/Output Functions *
 **************************/

static int basic_print(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooUser *user;
	MooObject *obj;
	MooThing *channel;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (!(user = dynamic_cast<MooUser *>(env->get("user"))))
		throw moo_type_error;
	channel = dynamic_cast<MooThing *>(env->get("channel"));
	user->notify(TNT_STATUS, NULL, channel, buffer);
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
	for (int i = 0; i <= args->m_args->last(); i++) {
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
	for (int i = 0; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
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
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (args->m_args->get(i) == obj) {
			args->m_result = new MooNumber((moo_integer_t) 0);
			return(0);
		}
	}
	args->m_result = new MooNumber(1);
	return(0);
}
*/

/********************
 * String Functions *
 ********************/

static int basic_concat(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j = 0;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() < 0)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->m_args->last(); i++) {
		obj = args->m_args->get(i);
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	buffer[j] = '\0';
	args->m_result = new MooString(buffer);
	return(0);
}

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

static int basic_eval(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	return(frame->push_code(args->m_args->get_string(0)));
}

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
	return(frame->push_code(buffer));
}

static int basic_call_method(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooArgs *newargs;
	MooObject *obj;
	MooObjectArray *array;

	if (args->m_args->last() != 1 && args->m_args->last() != 2)
		throw moo_args_mismatched;
	obj = args->m_args->get(2);
	if (!obj || obj == &moo_nil)
		newargs = new MooArgs();
	else if (!(array = dynamic_cast<MooObjectArray *>(obj)))
		throw moo_type_error;
	else
		newargs = new MooArgs(array);
	newargs->m_this = dynamic_cast<MooThing *>(args->m_args->get(0));
	return(frame->push_call(env, args->m_args->get(1), newargs));
}

static int basic_throw(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	char buffer[STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	obj->to_string(buffer, STRING_SIZE);
	throw MooException("%s", buffer);
}

int moo_load_basic_funcs(MooObjectHash *env)
{
	env->set("print", new MooFunc(basic_print));
	env->set("format", new MooFunc(basic_format));

	env->set("+", new MooFunc(basic_add));
	env->set("-", new MooFunc(basic_subtract));
	env->set("*", new MooFunc(basic_multiply));
	env->set("/", new MooFunc(basic_divide));

	env->set("null", new MooFunc(basic_null));
	env->set("eqv", new MooFunc(basic_eqv));
	env->set("=", new MooFunc(basic_equal));
	env->set("!=", new MooFunc(basic_not_equal));
	//env->set(">", new MooFunc(basic_gt));
	//env->set(">=", new MooFunc(basic_ge));
	//env->set("<", new MooFunc(basic_lt));
	//env->set("<=", new MooFunc(basic_le));

	//env->set("not", new MooFunc(basic_not));

	env->set("concat", new MooFunc(basic_concat));

	env->set("array", new MooFunc(basic_array));
	env->set("hash", new MooFunc(basic_hash));

	env->set("@eval", new MooFunc(basic_eval));
	env->set("@load", new MooFunc(basic_load));

	env->set("call-method", new MooFunc(basic_call_method));
	env->set("throw", new MooFunc(basic_throw));
	return(0);
}


