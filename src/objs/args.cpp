/*
 * Object Name:	args.c
 * Description:	Args Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/objs/object.h>
#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/user.h>
#include <sdm/objs/args.h>

#define MOO_IS_WHITESPACE(ch)	( (ch) == ' ' || (ch) == '\n' || (ch) == '\r' )

static char *moo_prepositions[] = { "from", "on", "with", "to", "at", NULL };


MooObjectType moo_args_obj_type = {
	NULL,
	"args",
	typeid(MooArgs).name(),
	(moo_type_create_t) moo_args_create
};

MooObject *moo_args_create(void)
{
	return(new MooArgs());
}

MooArgs::MooArgs(int init_size, MooThing *user, MooThing *channel)
{
	this->init(user, channel, NULL);
	m_args = new MooObjectArray(init_size);
}

MooArgs::MooArgs(MooObjectArray *args, MooThing *user, MooThing *channel)
{
	this->init(user, channel, NULL);
	MOO_INCREF(m_args = args);
}

MooArgs::~MooArgs()
{
	MOO_DECREF(m_result);
	MOO_DECREF(m_args);
}

void MooArgs::init(MooThing *user, MooThing *channel, MooObject *thing)
{
	if (!user)
		user = MooThing::lookup(MooTask::current_user());
	m_result = NULL;
	m_user = user;
	m_channel = channel;
	m_this = thing;
}

void MooArgs::set_args(MooObjectArray *&args)
{
	MOO_DECREF(m_args);
	m_args = args;
	args = NULL;
}

int MooArgs::find_whitespace(const char *text)
{
	int i;

	for (i = 0; text[i] != '\0' && !MOO_IS_WHITESPACE(text[i]); i++)
		;
	return(i);
}

int MooArgs::find_character(const char *text)
{
	int i;

	for (i = 0; text[i] != '\0' && MOO_IS_WHITESPACE(text[i]); i++)
		;
	return(i);
}

const char *MooArgs::parse_word(char *buffer, int max, const char *text)
{
	char *remain;

	strncpy(buffer, text, max);
	buffer[max - 1] = '\0';
	remain = MooArgs::parse_word(buffer);
	return(&text[remain - buffer]);
}

char *MooArgs::parse_word(char *buffer)
{
	int i;

	/// Get rid of any whitespace at the start of the string (??)
	i = MooArgs::find_character(buffer);
	if (buffer[i] == '\"') {
		for (i++; buffer[i] != '\0' && buffer[i] != '\"'; i++)
			;
	}
	else {
		/// Find the first word, ending in a space, and isolate it
		i += MooArgs::find_whitespace(&buffer[i]);
	}

	if (buffer[i] != '\0') {
		buffer[i++] = '\0';
		/// Find the start of the next word and return this as the arguments string
		i += MooArgs::find_character(&buffer[i]);
	}
	return(&buffer[i]);
}

#define MAX_STACK	10

int MooArgs::parse_args(const char *params, const char *text)
{
	int i, j = 0, k, sp = 0, ap = 0;
	MooObject *obj;
	const MooObjectType *type;
	char stack[MAX_STACK];
	char buffer[LARGE_STRING_SIZE];

	strncpy(buffer, text, LARGE_STRING_SIZE);

	stack[sp] = '\0';
	j += MooArgs::find_character(&buffer[j]);
	for (i = 0; params[i] != '\0'; i++) {
		if (params[i] == stack[sp])
			sp--;
		else if (params[i] == '[')
			stack[++sp] = ']';
		else {
			if (&buffer[j] == '\0')
				break;
			if (params[i] == 's') {
				obj = new MooString(&buffer[j]);
				k = ((MooString *) obj)->m_len;
			}
			else {
				if (!(type = MooArgs::get_type(params[i])))
					throw MooException("Error: Invalid arg type (%d), expected %c", ap, params[i]);
				if (!(obj = moo_make_object(type)))
					throw moo_mem_error;
				k = obj->parse_arg(NULL, NULL, &buffer[j]);
			}
			if (!k) {
				delete obj;
				if (stack[sp] == ']') {
					while (params[i] != '\0' && params[i] != stack[sp])
						i++;
				}
				else
					throw MooException("Error parsing argument (%s)", type->m_name);
			}
			else {
				j += k;
				m_args->set(ap++, obj);
				/// Trim any whitespace before next argument
				k = MooArgs::find_character(&buffer[j]);
				/// If we didn't find any whitespace then the argument was invalid
				if (!k && buffer[j] != '\0')
					throw MooException("Error: Invalid argument (%s)", type->m_name);
				j += k;
			}
		}
	}
	for (; params[i] != '\0'; i++) {
		if (params[i] == stack[sp])
			sp--;
		else if (params[i] == '[')
			stack[++sp] = ']';
		else if (sp <= 0)
			throw MooException("Error: Not enough arguments");
	}
	if (buffer[j] != '\0')
		throw MooException("Error: Too many arguments");
	return(0);
}

const MooObjectType *MooArgs::get_type(char param)
{
	switch (param) {
	    case 's':
	    case 'w':
		return(&moo_string_obj_type);
	    case 'i':
		return(&moo_integer_obj_type);
	    case 'f':
		return(&moo_float_obj_type);
	    case 't':
		return(&moo_thingref_obj_type);
	    case 'a':
		return(&moo_array_obj_type);
	    case 'h':
		return(&moo_hash_obj_type);
	    default:
		return(NULL);
	}
}


MooObject *MooArgs::access_property(const char *name, MooObject *value)
{
	if (!strcmp(name, "args")) {
		MOO_SET_MEMBER(m_args, MooObjectArray *, value)
		return(m_args);
	}
	else if (!strcmp(name, "this")) {
		MOO_SET_MEMBER(m_this, MooThing *, value)
		return(m_this);
	}
	else if (!strcmp(name, "result")) {
		if (value)
			m_result = value;
		return(m_result);
	}
	else if (!strcmp(name, "user")) {
		MOO_SET_MEMBER(m_user, MooThing *, value)
		return(m_user);
	}
	else if (!strcmp(name, "channel")) {
		MOO_SET_MEMBER(m_channel, MooThing *, value)
		return(m_channel);
	}
	return(NULL);
}

MooObjectHash *MooArgs::make_env(MooObjectHash *env)
{
	if (!env)
		env = new MooObjectHash();
	env->set("user", MOO_INCREF(m_user));
	env->set("channel", MOO_INCREF(m_channel));
	env->set("this", MOO_INCREF(m_this));
	env->set("result", MOO_INCREF(m_result));
	env->set("args", MOO_INCREF(m_args));
	return(env);
}

