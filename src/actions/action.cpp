/*
 * Object Name:	action.c
 * Description:	Action Base Object
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
#include <sdm/actions/action.h>

#define MOO_IS_WHITESPACE(ch)	( (ch) == ' ' || (ch) == '\n' || (ch) == '\r' )

static char *moo_prepositions[] = { "from", "on", "with", "to", "at", NULL };


MooObjectType moo_action_obj_type = {
	NULL,
	"action",
	typeid(MooAction).name(),
	(moo_type_create_t) NULL
};

MooAction::MooAction(const char *name, MooThing *thing)
{
	this->init(name, thing);
	m_params[0] = '\0';
}

MooAction::~MooAction()
{
	// TODO maybe you could check the thing and make sure the action is removed???
	if (m_name)
		delete m_name;
}

void MooAction::init(const char *name, MooThing *thing)
{
	m_name = name ? new std::string(name) : NULL;
	m_thing = thing;
}

const char *MooAction::params(const char *params)
{
	if (params) {
		strncpy(m_params, params, MOO_PARAM_STRING_SIZE);
		m_params[MOO_PARAM_STRING_SIZE - 1] = '\0';
	}
	return(m_params);
}


MooArgs::MooArgs(int init_size, MooThing *user, MooThing *channel)
{
	m_action = NULL;
	m_action_text = NULL;
	m_result = NULL;
	m_user = user;
	m_channel = channel;
	m_caller = NULL;
	m_this = NULL;
	m_args = new MooObjectArray(init_size);
}

MooArgs::MooArgs(MooObjectArray *&args, MooThing *user, MooThing *channel)
{
	m_action = NULL;
	m_action_text = NULL;
	m_result = NULL;
	m_user = user;
	m_channel = channel;
	m_caller = NULL;
	m_this = NULL;

	m_args = args;
	args = NULL;
}

MooArgs::MooArgs(MooArgs *args, int init_size)
{
	m_action = args->m_action;
	m_action_text = args->m_action_text;
	m_result = NULL;
	m_user = args->m_user;
	m_channel = args->m_channel;
	m_caller = args->m_caller;
	m_this = args->m_this;
	m_args = new MooObjectArray(init_size);
}

MooArgs::~MooArgs()
{
	if (m_result)
		delete m_result;
	if (m_args)
		delete m_args;
}

void MooArgs::set_args(MooObjectArray *&args)
{
	if (m_args)
		delete m_args;
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

void MooArgs::init(MooThing *user, MooThing *channel)
{
	m_user = user;
	m_channel = channel;
	// TODO should this even be used?
	//m_caller = (MooThing *) user;
}

int MooArgs::parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer, int max, const char *text)
{
	strncpy(buffer, text, max);
	return(this->parse_args(params, user, channel, buffer));
}

#define MAX_STACK	10

int MooArgs::parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer)
{
	int i, j = 0, k, sp = 0, ap = 0;
	MooObject *obj;
	const MooObjectType *type;
	char stack[MAX_STACK];

	this->init(user, channel);

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
					throw moo_args_error;
				if (!(obj = moo_make_object(type)))
					throw moo_mem_error;
				k = obj->parse_arg(user, channel, &buffer[j]);
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

int MooArgs::match_args(const char *params)
{
	int i;
	MooObject *obj;

	// TODO fix this so that it supports variable args parsing
	for (i = 0; i <= m_args->last(); i++) {
		if (params[i] == '\0')
			return(-1);
		if (!(obj = m_args->get(i, NULL)))
			return(-1);
		if (obj->type() != MooArgs::get_type(params[i]))
			return(-1);
	}
	if (params[i] != '\0')
		return(-1);
	return(0);
}

void MooArgs::match_args_throw(const char *params)
{
	if (this->match_args(params) < 0)
		throw moo_args_error;
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

