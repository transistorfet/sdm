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
#include <sdm/tasks/task.h>
#include <sdm/objs/thing.h>


/**************************
 * Input/Output Functions *
 **************************/

static int basic_print(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j = 0;
	MooObject *obj;
	MooThing *user, *channel;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() == -1)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->m_args->last(); i++) {
		if (!(obj = args->m_args->get(i)))
			throw moo_type_error;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	if (!(user = dynamic_cast<MooThing *>(env->get("user"))))
		throw moo_type_error;
	channel = dynamic_cast<MooThing *>(env->get("channel"));
	user->notify(TNT_STATUS, NULL, channel, buffer);
	return(0);
}

static int basic_debug(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j = 0;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() == -1)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->m_args->last(); i++) {
		if (!(obj = args->m_args->get(i)))
			throw moo_type_error;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	moo_status("DEBUG: %s", buffer);
	return(0);
}

static int basic_printstack(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != -1)
		throw moo_args_mismatched;
	frame->print_stacktrace();
	return(0);
}


/******************
 * Math Functions *
 ******************/

static int basic_add(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *result, *num;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	result = new MooNumber((long int) 0);
	for (int i = 0; i <= args->m_args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->add(num);
	}
	args->m_result = result;
	return(0);
}

static int basic_subtract(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *result, *num;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->subtract(num);
	}
	args->m_result = result;
	return(0);
}

static int basic_multiply(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *result, *num;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->multiply(num);
	}
	args->m_result = result;
	return(0);
}

static int basic_divide(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *result, *num;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number", 0);
	result = new MooNumber(num);
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		result->divide(num);
	}
	args->m_result = result;
	return(0);
}

/************************
 * Comparison Functions *
 ************************/

static int basic_null(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	if (!obj || (obj == &moo_nil)) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
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
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_not_eqv(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (args->m_args->get(i) != obj) {
			args->m_result = new MooBoolean(B_TRUE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	char buffer1[LARGE_STRING_SIZE];
	char buffer2[LARGE_STRING_SIZE];

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;

	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	obj->to_string(buffer1, LARGE_STRING_SIZE);

	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(obj = args->m_args->get(i)))
			throw moo_type_error;
		obj->to_string(buffer2, LARGE_STRING_SIZE);
		if (strcmp(buffer1, buffer2)) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_num_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2)) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_num_not_equal(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (!num1->compare(num2)) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_gt(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) != 1) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_ge(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) == -1) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_lt(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) != -1) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_le(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooNumber *num1, *num2;

	if (args->m_args->last() < 1)
		throw moo_args_mismatched;
	if (!(num1 = dynamic_cast<MooNumber *>(args->m_args->get(0))))
		throw MooException("arg 1: Invalid type. Expected number");
	for (int i = 1; i <= args->m_args->last(); i++) {
		if (!(num2 = dynamic_cast<MooNumber *>(args->m_args->get(i))))
			throw MooException("arg %d: Invalid type. Expected number", i + 1);
		if (num1->compare(num2) == 1) {
			args->m_result = new MooBoolean(B_FALSE);
			return(0);
		}
	}
	args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

static int basic_not(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)) || obj->is_true())
		args->m_result = new MooBoolean(B_FALSE);
	else
		args->m_result = new MooBoolean(B_TRUE);
	return(0);
}

/********************
 * String Functions *
 ********************/

static int basic_expand(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *str;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	str = args->m_args->get_string(0);
	MooObject::format(buffer, LARGE_STRING_SIZE, env, str);
	args->m_result = new MooString("%s", buffer);
	return(0);
}

static int basic_strlen(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooString *str;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(str = dynamic_cast<MooString *>(args->m_args->get(0))))
		throw moo_type_error;
	args->m_result = new MooNumber((long int) str->m_len);
	return(0);
}

static int basic_concat(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j = 0;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() < 0)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->m_args->last() && j < LARGE_STRING_SIZE; i++) {
		// TODO temporary? how the hell does/should nil/null work?
		if (!(obj = args->m_args->get(i)))
			obj = &moo_nil;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	buffer[j] = '\0';
	args->m_result = new MooString("%s", buffer);
	return(0);
}

static int basic_chop(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int i;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	i = obj->to_string(buffer, LARGE_STRING_SIZE);
	if (--i >= 0)
		buffer[i] = '\0';
	args->m_result = new MooString("%s", buffer);
	return(0);
}

static int basic_substr(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j, k;
	MooObject *obj;
	MooNumber *num;
	int pos, len = -1;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 1 && args->m_args->last() != 2)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;

	if ((num = dynamic_cast<MooNumber *>(args->m_args->get(1))))
		pos = num->get_integer();
	else
		throw moo_type_error;
	if (pos < 0) {
		args->m_result = new MooString("");
		return(0);
	}

	if (args->m_args->last() == 2) {
		if ((num = dynamic_cast<MooNumber *>(args->m_args->get(2))))
			len = num->get_integer();
		else
			throw moo_type_error;
	}
	obj->to_string(buffer, LARGE_STRING_SIZE);
	for (j = pos, k = 0; buffer[j] != '\0' && k < LARGE_STRING_SIZE && (len == -1 || k < len); j++, k++)
		buffer[k] = buffer[j];
	buffer[k] = '\0';
	args->m_result = new MooString("%s", buffer);
	return(0);
}

static int basic_ltrim(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int i;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	obj->to_string(buffer, LARGE_STRING_SIZE);
	for (i = 0; buffer[i] != '\0' && (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n' || buffer[i] == '\r'); i++)
		;
	args->m_result = new MooString("%s", &buffer[i]);
	return(0);
}

/******************
 * Type Functions *
 ******************/

static int basic_type(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	args->m_result = new MooString("%s", obj->objtype_name());
	return(0);
}

static int basic_boolean_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooBoolean *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_number_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooNumber *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_string_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooString *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_array_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooObjectArray *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_hash_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooObjectHash *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_thing_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooThing *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_lambda_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooCodeLambda *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}

static int basic_method_q(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (dynamic_cast<MooMethod *>(args->m_args->get(0))) {
		args->m_result = new MooBoolean(B_TRUE);
		return(0);
	}
	args->m_result = new MooBoolean(B_FALSE);
	return(0);
}


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

#define MAX_FILE_SIZE	65535

static int basic_load(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int len;
	MooObject *obj;
	char buffer[MAX_FILE_SIZE];

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		return(-1);
	obj->to_string(buffer, MAX_FILE_SIZE);
	len = moo_data_read_file(buffer, buffer, MAX_FILE_SIZE);
	if (len <= 0)
		throw MooException("Unable to read file %s", buffer);
	else if (len >= MAX_FILE_SIZE)
		throw MooException("File exceeds maximum file size of %d bytes", MAX_FILE_SIZE);
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

static int basic_return(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() > 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	args->m_result = obj;
	frame->goto_return_point();
	return(0);
}

static int basic_perms(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	args->m_result = new MooNumber((long int) obj->permissions());
	return(0);
}

static int basic_owner(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;

	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)))
		throw moo_type_error;
	args->m_result = new MooNumber((long int) obj->owner());
	return(0);
}

static int basic_chmod(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *owner = NULL;
	MooObject *obj, *perms;

	if (args->m_args->last() == 1)
		obj = args->m_args->get(1);
	else if (args->m_args->last() == 2) {
		if ((obj = args->m_args->get(1)) && !(owner = dynamic_cast<MooThing *>(obj)))
			throw moo_type_error;
		obj = args->m_args->get(2);
	}
	else
		throw moo_args_mismatched;

	obj->check_throw(MOO_PERM_W);
	if ((perms = args->m_args->get(0)))
		obj->permissions(perms->get_integer());
	if (owner)
		obj->owner(owner->id());
	args->m_result = obj;
	return(0);
}

int moo_load_basic_funcs(MooObjectHash *env)
{
	env->set("print", new MooFunc(basic_print));
	env->set("debug", new MooFunc(basic_debug));
	env->set("printstack", new MooFunc(basic_printstack));

	env->set("+", new MooFunc(basic_add));
	env->set("-", new MooFunc(basic_subtract));
	env->set("*", new MooFunc(basic_multiply));
	env->set("/", new MooFunc(basic_divide));

	env->set("null?", new MooFunc(basic_null));
	env->set("eqv?", new MooFunc(basic_eqv));
	env->set("!eqv?", new MooFunc(basic_not_eqv));
	env->set("equal?", new MooFunc(basic_equal));
	env->set("=", new MooFunc(basic_num_equal));
	env->set("!=", new MooFunc(basic_num_not_equal));
	env->set(">", new MooFunc(basic_gt));
	env->set(">=", new MooFunc(basic_ge));
	env->set("<", new MooFunc(basic_lt));
	env->set("<=", new MooFunc(basic_le));

	env->set("not", new MooFunc(basic_not));

	env->set("expand", new MooFunc(basic_expand));
	env->set("strlen", new MooFunc(basic_strlen));
	env->set("concat", new MooFunc(basic_concat));
	env->set("chop", new MooFunc(basic_chop));
	env->set("substr", new MooFunc(basic_substr));
	env->set("ltrim", new MooFunc(basic_ltrim));

	env->set("type", new MooFunc(basic_type));
	env->set("boolean?", new MooFunc(basic_boolean_q));
	env->set("number?", new MooFunc(basic_number_q));
	env->set("string?", new MooFunc(basic_string_q));
	env->set("array?", new MooFunc(basic_array_q));
	env->set("hash?", new MooFunc(basic_hash_q));
	env->set("thing?", new MooFunc(basic_thing_q));
	env->set("lambda?", new MooFunc(basic_lambda_q));
	env->set("method?", new MooFunc(basic_method_q));

	env->set("array", new MooFunc(basic_array));
	env->set("hash", new MooFunc(basic_hash));

	env->set("eval", new MooFunc(basic_eval));
	env->set("load", new MooFunc(basic_load));

	env->set("call-method", new MooFunc(basic_call_method));
	env->set("throw", new MooFunc(basic_throw));
	env->set("return", new MooFunc(basic_return));

	env->set("perms", new MooFunc(basic_perms));
	env->set("owner", new MooFunc(basic_owner));
	env->set("chmod", new MooFunc(basic_chmod));

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


