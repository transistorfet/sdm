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
	MooObject *obj, *func;
	MooObjectArray *newargs;
	MooThing *user, *channel;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() == -1)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->last(); i++) {
		if (!(obj = args->get(i)))
			throw moo_type_error;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	env = frame->env();
	if (!(user = dynamic_cast<MooThing *>(env->get("user"))))
		throw moo_type_error;

	if (!(func = user->resolve_method("tell")))
		throw MooException("User object does not have a \'tell\' method");
	newargs = new MooObjectArray();
	args->set(0, user);
	args->set(1, new MooString("%s", buffer));
	frame->push_call(env, func, args);
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

/********************
 * String Functions *
 ********************/

static int basic_expand(MooCodeFrame *frame, MooObjectArray *args)
{
	const char *str;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	str = args->get_string(0);
	MooObject::format(buffer, LARGE_STRING_SIZE, frame->env(), str);
	frame->set_return(new MooString("%s", buffer));
	return(0);
}

static int basic_strlen(MooCodeFrame *frame, MooObjectArray *args)
{
	MooString *str;

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(str = dynamic_cast<MooString *>(args->get(0))))
		throw moo_type_error;
	frame->set_return(new MooNumber((long int) str->length()));
	return(0);
}

static int basic_concat(MooCodeFrame *frame, MooObjectArray *args)
{
	int j = 0;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() < 0)
		throw moo_args_mismatched;
	for (int i = 0; i <= args->last() && j < LARGE_STRING_SIZE; i++) {
		// TODO this is a really weird use of nil/NULL; all undefined values will be stringified as 'nil'.
		//	We could alternatively stringify undefined values as 'undef' or something
		if (!(obj = args->get(i)))
			obj = &moo_nil;
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	buffer[j] = '\0';
	frame->set_return(new MooString("%s", buffer));
	return(0);
}

static int basic_chop(MooCodeFrame *frame, MooObjectArray *args)
{
	int i;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)))
		throw moo_type_error;
	i = obj->to_string(buffer, LARGE_STRING_SIZE);
	if (--i >= 0)
		buffer[i] = '\0';
	frame->set_return(new MooString("%s", buffer));
	return(0);
}

static int basic_substr(MooCodeFrame *frame, MooObjectArray *args)
{
	int j, k;
	MooObject *obj;
	MooNumber *num;
	int pos, len = -1;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 1 && args->last() != 2)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)))
		throw moo_type_error;

	if ((num = dynamic_cast<MooNumber *>(args->get(1))))
		pos = num->get_integer();
	else
		throw moo_type_error;
	if (pos < 0) {
		frame->set_return(new MooString(""));
		return(0);
	}

	if (args->last() == 2) {
		if ((num = dynamic_cast<MooNumber *>(args->get(2))))
			len = num->get_integer();
		else
			throw moo_type_error;
	}
	obj->to_string(buffer, LARGE_STRING_SIZE);
	for (j = pos, k = 0; buffer[j] != '\0' && k < LARGE_STRING_SIZE && (len == -1 || k < len); j++, k++)
		buffer[k] = buffer[j];
	buffer[k] = '\0';
	frame->set_return(new MooString("%s", buffer));
	return(0);
}

static int basic_ltrim(MooCodeFrame *frame, MooObjectArray *args)
{
	int i;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)))
		throw moo_type_error;
	obj->to_string(buffer, LARGE_STRING_SIZE);
	for (i = 0; buffer[i] != '\0' && (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n' || buffer[i] == '\r'); i++)
		;
	frame->set_return(new MooString("%s", &buffer[i]));
	return(0);
}

#define MAX_WORDS	256

static int basic_parse_words(MooCodeFrame *frame, MooObjectArray *args)
{
	int i = 0, j = 0;
	MooObjectArray *array;
	const char *text;
	char *words[MAX_WORDS];
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 0)
		throw moo_args_mismatched;
	text = args->get_string(0);

	/// Parse the text into words
	strncpy(buffer, text, LARGE_STRING_SIZE);
	buffer[LARGE_STRING_SIZE] = '\0';
	while (parser_is_whitespace(buffer[i]))
		i++;
	words[0] = &buffer[i];
	for (; buffer[i] != '\0'; i++) {
		if (buffer[i] == '\"') {
			/// If the quote isn't the start of the next word, then close off the old word and start a new one
			if (words[j][0] != '\"') {
				buffer[i] = '\0';
				j++;
			}

			/// Remove the quote from the word (or otherwise, start a new word)
			words[j] = &buffer[++i];
			for (; buffer[i] != '\0' && buffer[i] != '\"'; i++)
				;
		}

		if (buffer[i] == '\"' || parser_is_whitespace(buffer[i])) {
			buffer[i++] = '\0';
			while (parser_is_whitespace(buffer[i]))
				i++;
			words[++j] = &buffer[i];
			i--;
		}
	}
	if (*words[j] == '\0')
		j--;

	/// Build the array
	array = new MooObjectArray();
	for (int k = 0; k <= j; k++)
		array->push(new MooString("%s", words[k]));
	frame->set_return(array);
	return(0);
}

static int basic_remove_word(MooCodeFrame *frame, MooObjectArray *args)
{
	int i = 0;
	const char *text;

	if (args->last() != 0)
		throw moo_args_mismatched;
	text = args->get_string(0);

	while (text[i] != '\0' && parser_is_whitespace(text[i]))
		i++;
	while (text[i] != '\0' && !parser_is_whitespace(text[i]))
		i++;
	while (text[i] != '\0' && parser_is_whitespace(text[i]))
		i++;
	frame->set_return(new MooString("%s", &text[i]));
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

	env->set("expand", new MooFuncPtr(basic_expand));
	env->set("strlen", new MooFuncPtr(basic_strlen));
	env->set("concat", new MooFuncPtr(basic_concat));
	env->set("chop", new MooFuncPtr(basic_chop));
	env->set("substr", new MooFuncPtr(basic_substr));
	env->set("ltrim", new MooFuncPtr(basic_ltrim));
	env->set("parse-words", new MooFuncPtr(basic_parse_words));
	env->set("remove-word", new MooFuncPtr(basic_remove_word));

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


