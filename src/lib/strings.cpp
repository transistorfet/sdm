/*
 * Name:	strings.cpp
 * Description:	String Functions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

int string_expand(char *buffer, int max, MooObjectHash *env, const char *fmt);
int string_expand_reference(char *buffer, int max, MooObjectHash *env, const char *str, int *used);
int string_escape_char(const char *str, char *buffer);

///// String Parsers /////

/**
 * Format a string using the given fmt string and place the resulting
 * string into the given buffer.  The number of characters written to
 * the buffer is returned.  If a $ is encountered, the substring up
 * to the next space is taken to be a reference name.  If the reference
 * name is enclosed in { }, then the reference name is take to be
 * up to the closing }.  The reference is evaluated into a string using
 * the MooObject::expand_reference function.  If a & follows the $ then the
 * resolved string is recursively expanded.
 */
int string_expand(char *buffer, int max, MooObjectHash *env, const char *fmt)
{
	int i;
	int j = 0;

	max--;
	for (i = 0;(fmt[i] != '\0') && (j < max);i++) {
		if (fmt[i] == '\\') {
			if (fmt[++i] == '\0')
				break;
			i += string_escape_char(&fmt[i], &buffer[j++]) - 1;
		}
		else if (fmt[i] == '$')
			j += string_expand_reference(&buffer[j], max - j + 1, env, &fmt[i], &i);
		else
			buffer[j++] = fmt[i];
	}
	buffer[j] = '\0';
	return(j);
}

#define IS_NUMBER_CHAR(ch)	\
	( ((ch) >= 0x30) && ((ch) <= 0x39) )

#define IS_VALID_CHAR(ch)	\
	( (((ch) >= '0') && ((ch) <= '9'))	\
	|| (((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'z'))	\
	|| ((ch) == '.') || ((ch) == '_') )

/**
 * Expand the reference using the given args and copy the resulting string
 * to the given buffer up to the max number of characters.  The number
 * of characters written to the buffer is returned.  If the given str_count
 * pointer is not NULL then the number of characters used as the variable
 * name in str is added to the value it points to.  The given str may
 * start with the '$' char or may start just after it.
 */
int string_expand_reference(char *buffer, int max, MooObjectHash *env, const char *str, int *used)
{
	int k;
	char delim;
	int recurse;
	int i = 0, j = 0;
	MooObject *obj;

	if (str[i] == '$')
		i++;

	if (str[i] == '&') {
		recurse = 1;
		i++;
	}
	else
		recurse = 0;

	if (str[i] == '{') {
		delim = '}';
		i++;
	}
	else
		delim = ' ';

	for (k = 0; (str[i] != '\0') && (str[i] != delim) && IS_VALID_CHAR(str[i]) && (k < max - 1); k++, i++)
		buffer[k] = str[i];
	buffer[k] = '\0';
	// TODO this should be replaced by frame->resolve()
	if ((obj = env->get(buffer))) {
		j = obj->to_string(buffer, max);
		if (recurse) {
			char value[STRING_SIZE];
			strncpy(value, buffer, STRING_SIZE);
			value[STRING_SIZE - 1] = '\0';
			j = string_expand(buffer, max, env, value);
		}
	}
	if (delim != '}')
		i--;

	if (used)
		*used += i;
	buffer[j] = '\0';
	return(j);
}


/**
 * Convert the charcter escape sequence (assuming str starts after the escape
 * character) and stores the character in buffer[0].  The number of characters
 * read as a sequence from str is returned
 */
int string_escape_char(const char *str, char *buffer)
{
	char number[3];

	if (*str == '\0')
		return(0);
	switch (str[0]) {
		case 'n':
			buffer[0] = '\n';
			break;
		case 'r':
			buffer[0] = '\r';
			break;
		case 't':
			buffer[0] = '\t';
			break;
		case 'e':
			buffer[0] = '\x1b';
			break;
		case 'x':
			if ((str[1] != '\0') && (str[2] != '\0')) {
				number[0] = str[1];
				number[1] = str[2];
				number[2] = '\0';
				buffer[0] = strtol(number, NULL, 16);
			}
			return(3);
		default:
			if (IS_NUMBER_CHAR(str[0])) {
				buffer[0] = str[0] - 0x30;
				if (IS_NUMBER_CHAR(str[1])) {
					buffer[0] = (buffer[0] * 8) + str[1] - 0x30;
					if (IS_NUMBER_CHAR(str[2])) {
						buffer[0] = (buffer[0] * 8) + str[2] - 0x30;
						return(3);
					}
					return(2);
				}
				return(1);
			}
			else
				buffer[0] = str[0];
			break;
	}
	return(1);
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
	string_expand(buffer, LARGE_STRING_SIZE, frame->env(), str);
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

int moo_load_string_funcs(MooObjectHash *env)
{
	env->set("expand", new MooFuncPtr(basic_expand));
	env->set("strlen", new MooFuncPtr(basic_strlen));
	env->set("concat", new MooFuncPtr(basic_concat));
	env->set("chop", new MooFuncPtr(basic_chop));
	env->set("substr", new MooFuncPtr(basic_substr));
	env->set("ltrim", new MooFuncPtr(basic_ltrim));
	env->set("parse-words", new MooFuncPtr(basic_parse_words));
	env->set("remove-word", new MooFuncPtr(basic_remove_word));
	return(0);
}


