/*
 * Name:	utils.c
 * Description:	Thing Utilities
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/utils.h>
#include <sdm/processors/interpreter.h>

#define IS_NUMBER_CHAR(ch)	\
	( ((ch) >= 0x30) && ((ch) <= 0x39) )

#define IS_VALID_CHAR(ch)	\
	( (((ch) >= '0') && ((ch) <= '9'))	\
	|| (((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'z'))	\
	|| ((ch) == '.') || ((ch) == '_') )

int sdm_do_format_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, const char *fmt, ...)
{
	int i;
	va_list va;
	char buffer[STRING_SIZE];
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = buffer;

	va_start(va, fmt);
	if ((i = vsnprintf(buffer, STRING_SIZE - 1, fmt, va)) < 0)
		return(-1);
	if (i >= STRING_SIZE - 1)
		buffer[STRING_SIZE - 1] = '\0';
	return(sdm_thing_do_action(thing, action, &args));
}

int sdm_do_expand_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, struct sdm_action_args *args, const char *fmt, ...)
{
	va_list va;
	char buffer[STRING_SIZE];
	struct sdm_action_args newargs;

	memset(&newargs, '\0', sizeof(struct sdm_action_args));
	newargs.caller = caller;
	newargs.text = buffer;

	va_start(va, fmt);
	{
		int i;
		char tmp[STRING_SIZE];

		if ((i = vsnprintf(tmp, STRING_SIZE - 1, fmt, va)) < 0)
			return(-1);
		if (i >= STRING_SIZE - 1)
			buffer[STRING_SIZE - 1] = '\0';
		sdm_util_expand_str(buffer, STRING_SIZE - 1, args, tmp);
	}
	va_end(va);
	return(sdm_thing_do_action(thing, action, &newargs));
}

/**
 * Format a string using the given fmt string and place the resulting
 * string into the given buffer.  The number of characters written to
 * the buffer is returned.  If a $ is encountered, the substring up
 * to the next space is taken to be a reference name.  If the reference
 * name is enclosed in { }, then the reference name is take to be
 * up to the closing }.  The reference is evaluated into a string using
 * the sdm_util_expand_reference function.  If a & follows the $ then the
 * resolved string is recursively expanded.
 */
int sdm_util_expand_str(char *buffer, int max, struct sdm_action_args *args, const char *fmt)
{
	int i;
	int j = 0;

	max--;
	for (i = 0;(fmt[i] != '\0') && (j < max);i++) {
		if (fmt[i] == '\\') {
			if (fmt[++i] == '\0')
				break;
			i += sdm_util_escape_char(&fmt[i], &buffer[j++]) - 1;
		}
		else if (fmt[i] == '$')
			j += sdm_util_expand_reference(&buffer[j], max - j + 1, args, &fmt[i], &i);
		else
			buffer[j++] = fmt[i];
	}
	buffer[j] = '\0';
	return(j);
}

/**
 * Expand the reference using the given args and copy the resulting string
 * to the given buffer up to the max number of characters.  The number
 * of characters written to the buffer is returned.  If the given str_count
 * pointer is not NULL then the number of characters used as the variable
 * name in str is added to the value it points to.  The given str may
 * start with the '$' char or may start just after it.
 */
int sdm_util_expand_reference(char *buffer, int max, struct sdm_action_args *args, const char *str, int *used)
{
	int k;
	char delim;
	int recurse;
	int i = 0, j = 0;
	char value[STRING_SIZE];

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

	for (k = 0;(str[i] != '\0') && (str[i] != delim) && IS_VALID_CHAR(str[i]) && (k < max - 1);k++, i++)
		buffer[k] = str[i];
	buffer[k] = '\0';
	if (sdm_util_resolve_reference(value, STRING_SIZE, args, buffer) >= 0) {
		if (recurse)
			j = sdm_util_expand_str(buffer, max, args, value);
		else {
			strncpy(buffer, value, max - 1);
			if ((j = strlen(value)) >= max)
				j = max - 1;
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
 * Resolves the given reference using the given arguments and copies the
 * resulting string into the given buffer.  The number of characters written
 * to the buffer is returned.
 */
int sdm_util_resolve_reference(char *buffer, int max, struct sdm_action_args *args, const char *ref)
{
	int i;
	char *name;
	struct sdm_thing *thing;
	struct sdm_object *result;

	if ((name = strchr(ref, '.'))) {
		i = name - ref;
		name++;
	}
	else
		i = strlen(ref);

	if (!strncmp(ref, "text", i)) {
		// TODO we are ignoring the rest of the reference for now ($text.prop <=> $text)
		strncpy(buffer, args->text, max);
		return(strlen(args->text));
	}
	else if (!strncmp(ref, "action", i)) {
		strncpy(buffer, args->action, max);
		return(strlen(args->action));
	}
	else if (!strncmp(ref, "thing", i))
		thing = args->thing;
	else if (!strncmp(ref, "caller", i))
		thing = args->caller;
	else if (!strncmp(ref, "obj", i))
		thing = args->obj;
	else if (!strncmp(ref, "target", i))
		thing = args->target;
	// TODO allow direct references ( like #123.name and /core/thing.name )

	if (!name || !(result = sdm_thing_get_property(thing, name, NULL)))
		return(0);
	if (sdm_object_is_a(result, &sdm_string_obj_type)) {
		strncpy(buffer, SDM_STRING(result)->str, max);
		return(SDM_STRING(result)->len);
	}
	else if (sdm_object_is_a(result, &sdm_number_obj_type)) {
		if ((i = snprintf(buffer, max, "%f", SDM_NUMBER(result)->num)) < 0)
			return(0);
		return(i);
	}
	return(0);
}


/**
 * Convert the charcter escape sequence (assuming str starts after the escape
 * character) and stores the character in buffer[0].  The number of characters
 * read as a sequence from str is returned
 */
int sdm_util_escape_char(const char *str, char *buffer)
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
				buffer[0] = sdm_util_atoi(number, 16);
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

/**
 * Convert a string of the given radix to an interger.
 */
int sdm_util_atoi(const char *str, int radix)
{
	int i = -1, ret = 0, mul = 1;

	if (!str)
		return(0);

	while (str[++i] == ' ' || str[i] == '\t' || str[i] == '\n')
		if (str[i] == '\0')
			return(0);

	if (str[i] == '-') {
		mul = -1;
		i++;
	}

	for (;str[i] != '\0';i++) {
		ret *= radix;
		if (str[i] >= 0x30 && str[i] <= 0x39)
			ret += (str[i] - 0x30);
		else if (str[i] >= 0x41 && str[i] <= 0x5a)
			ret += (str[i] - 0x37);
		else if (str[i] >= 0x61 && str[i] <= 0x7a)
			ret += (str[i] - 0x57);
		else
			break;
	}
	return(ret * mul);
}

