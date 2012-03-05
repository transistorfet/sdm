/*
 * Name:	basic.cpp
 * Description:	Basic Functions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/objs/thing.h>


/**************************
 * Input/Output Functions *
 **************************/

static int basic_print(MooCodeFrame *frame, MooObjectArray *args)
{
	int j = 0;
	MooObjectHash *env;
	MooObject *obj, *out;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() == -1)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->last(); i++) {
		if (!(obj = args->get(i)))
			throw moo_type_error;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	env = frame->env();
	if (!(out = dynamic_cast<MooThing *>(env->get("*out*"))))
		throw MooException("No output object specified in *out*");
	frame->push_method_call("print", out, new MooString("%s", buffer));
	return(0);
}

static int basic_debug(MooCodeFrame *frame, MooObjectArray *args)
{
	int j = 0;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() == -1)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->last(); i++) {
		if (!(obj = args->get(i)))
			throw moo_type_error;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	moo_status("DEBUG: %s", buffer);
	return(0);
}

static int basic_printstack(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != -1)
		throw moo_args_mismatched;
	frame->print_stacktrace();
	return(0);
}


/******************
 * Math Functions *
 ******************/

static int basic_add(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *result, *num;

	if (args->last() < 1)
		throw moo_args_mismatched;
	result = new MooNumber((long int) 0);
	for (int i = 0; i <= args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->add(num);
	}
	frame->set_return(result);
	return(0);
}

static int basic_subtract(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *result, *num;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->subtract(num);
	}
	frame->set_return(result);
	return(0);
}

static int basic_multiply(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *result, *num;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->multiply(num);
	}
	frame->set_return(result);
	return(0);
}

static int basic_divide(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *result, *num;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->divide(num);
	}
	frame->set_return(result);
	return(0);
}

/************************
 * Comparison Functions *
 ************************/

static int basic_null(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() != 0)
		throw moo_args_mismatched;
	obj = args->get(0);
	if (MOO_IS_NIL(obj)) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_eqv(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() < 1)
		throw moo_args_mismatched;
	obj = args->get(0);
	for (int i = 1; i <= args->last(); i++) {
		if (args->get(i) != obj) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_not_eqv(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() < 1)
		throw moo_args_mismatched;
	obj = args->get(0);
	for (int i = 1; i <= args->last(); i++) {
		if (args->get(i) != obj) {
			frame->set_return(&moo_true);
			return(0);
		}
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_equal(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;
	char buffer1[LARGE_STRING_SIZE];
	char buffer2[LARGE_STRING_SIZE];

	if (args->last() < 1)
		throw moo_args_mismatched;

	if (!(obj = args->get(0)))
		throw moo_type_error;
	obj->to_string(buffer1, LARGE_STRING_SIZE);

	for (int i = 1; i <= args->last(); i++) {
		if (!(obj = args->get(i)))
			throw moo_type_error;
		obj->to_string(buffer2, LARGE_STRING_SIZE);
		if (strcmp(buffer1, buffer2)) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_num_equal(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2)) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_num_not_equal(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (!num1->compare(num2)) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_gt(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) != 1) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_ge(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) == -1) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_lt(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) != -1) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_le(MooCodeFrame *frame, MooObjectArray *args)
{
	MooNumber *num1, *num2;

	if (args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) == 1) {
			frame->set_return(&moo_false);
			return(0);
		}
	}
	frame->set_return(&moo_true);
	return(0);
}

static int basic_not(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)) || obj->is_true())
		frame->set_return(&moo_false);
	else
		frame->set_return(&moo_true);
	return(0);
}

/******************
 * Type Functions *
 ******************/

static int basic_type(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() != 0)
		throw moo_args_mismatched;
	obj = args->get(0);
	frame->set_return(new MooString("%s", obj->objtype_name()));
	return(0);
}

static int basic_boolean_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooBoolean *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_number_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooNumber *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_string_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooString *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_array_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooObjectArray *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_hash_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooObjectHash *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_thing_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooThing *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}

static int basic_lambda_q(MooCodeFrame *frame, MooObjectArray *args)
{
	if (args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooCodeLambda *>(args->get(0))) {
		frame->set_return(&moo_true);
		return(0);
	}
	frame->set_return(&moo_false);
	return(0);
}


static int basic_array(MooCodeFrame *frame, MooObjectArray *args)
{
	frame->set_return(args);
	return(0);
}

static int basic_hash(MooCodeFrame *frame, MooObjectArray *args)
{
	// TODO for now, we wont initialize the hash
	if (args->last() >= 0)
		throw moo_args_mismatched;
	frame->set_return(new MooObjectHash());
	return(0);
}

/********************
 * System Functions *
 ********************/

static int basic_eval(MooCodeFrame *frame, MooObjectArray *args)
{
	return(frame->push_code(args->get_string(0)));
}

#define MAX_FILE_SIZE	65535

static int basic_load(MooCodeFrame *frame, MooObjectArray *args)
{
	int len;
	MooObject *obj;
	char buffer[MAX_FILE_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)))
		return(-1);
	obj->to_string(buffer, MAX_FILE_SIZE);
	len = moo_data_read_file(buffer, buffer, MAX_FILE_SIZE);
	if (len <= 0)
		throw MooException("Unable to read file %s", buffer);
	else if (len >= MAX_FILE_SIZE)
		throw MooException("File exceeds maximum file size of %d bytes", MAX_FILE_SIZE);
	return(frame->push_code(buffer));
}

static int basic_get_property(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;
	const char *name;

	if (args->last() != 1)
		throw moo_args_mismatched;
	obj = args->get(0);
	name = args->get_string(1);
	frame->set_return(obj->resolve_property(name));
	return(0);
}

static int basic_get_method(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;
	const char *name;

	if (args->last() != 1)
		throw moo_args_mismatched;
	obj = args->get(0);
	name = args->get_string(1);
	frame->set_return(obj->resolve_method(name));
	return(0);
}

static int basic_throw(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;
	char buffer[STRING_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	obj = args->get(0);
	obj->to_string(buffer, STRING_SIZE);
	throw MooException("%s", buffer);
}

static int basic_return(MooCodeFrame *frame, MooObjectArray *args)
{
	MooObject *obj;

	if (args->last() > 0)
		throw moo_args_mismatched;
	obj = args->get(0);
	frame->set_return(obj);
	frame->goto_return_point();
	return(0);
}

static int basic_sleep(MooCodeFrame *frame, MooObjectArray *args)
{
	double time;

	if (args->last() != 0)
		throw moo_args_mismatched;
	time = args->get_float(0);
	frame->schedule(time);
	// TODO this should be redundant
	//throw MooCodeFrameSuspend();
}

static int basic_time(MooCodeFrame *frame, MooObjectArray *args)
{
	timeval tv;
	double time;

	if (args->last() != -1)
		throw moo_args_mismatched;
	gettimeofday(&tv, NULL);
	time = tv.tv_sec + (tv.tv_usec / 1000000.0);
	frame->set_return(new MooNumber(time));
	return(0);
}

int moo_load_basic_funcs(MooObjectHash *env)
{
	env->set("print", new MooFuncPtr(basic_print));
	env->set("debug", new MooFuncPtr(basic_debug));
	env->set("printstack", new MooFuncPtr(basic_printstack));

	env->set("+", new MooFuncPtr(basic_add));
	env->set("-", new MooFuncPtr(basic_subtract));
	env->set("*", new MooFuncPtr(basic_multiply));
	env->set("/", new MooFuncPtr(basic_divide));

	env->set("null?", new MooFuncPtr(basic_null));
	env->set("eqv?", new MooFuncPtr(basic_eqv));
	env->set("!eqv?", new MooFuncPtr(basic_not_eqv));
	env->set("equal?", new MooFuncPtr(basic_equal));
	env->set("=", new MooFuncPtr(basic_num_equal));
	env->set("!=", new MooFuncPtr(basic_num_not_equal));
	env->set(">", new MooFuncPtr(basic_gt));
	env->set(">=", new MooFuncPtr(basic_ge));
	env->set("<", new MooFuncPtr(basic_lt));
	env->set("<=", new MooFuncPtr(basic_le));

	env->set("not", new MooFuncPtr(basic_not));

	env->set("type", new MooFuncPtr(basic_type));
	env->set("boolean?", new MooFuncPtr(basic_boolean_q));
	env->set("number?", new MooFuncPtr(basic_number_q));
	env->set("string?", new MooFuncPtr(basic_string_q));
	env->set("array?", new MooFuncPtr(basic_array_q));
	env->set("hash?", new MooFuncPtr(basic_hash_q));
	env->set("thing?", new MooFuncPtr(basic_thing_q));
	env->set("lambda?", new MooFuncPtr(basic_lambda_q));

	env->set("array", new MooFuncPtr(basic_array));
	env->set("hash", new MooFuncPtr(basic_hash));

	env->set("eval", new MooFuncPtr(basic_eval));
	env->set("load", new MooFuncPtr(basic_load));

	env->set("get-property", new MooFuncPtr(basic_get_property));
	env->set("get-method", new MooFuncPtr(basic_get_method));
	env->set("throw", new MooFuncPtr(basic_throw));
	env->set("return", new MooFuncPtr(basic_return));
	env->set("sleep", new MooFuncPtr(basic_sleep));
	env->set("time", new MooFuncPtr(basic_time));

/*
	Possible Future Primatives:

	abs
	modulo
	floor
	ceiling
	round
	truncate

	sqrt
	exp
	expt
	log
	sin
	cos
	tan
	asin
	acos
	atan

	string-length

	(the whole character type)
	(the whole pairs type)
	(the whole symbols type)

*/

	return(0);
}


