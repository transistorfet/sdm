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

#include <sdm/code/code.h>

#include <sdm/things/user.h>

#define MOO_IS_WHITESPACE(ch)	( (ch) == ' ' || (ch) == '\n' || (ch) == '\r' )

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

int MooArgs::map_args(MooObjectHash *env, MooCodeExpr *params)
{
	int i;
	const char *id;
	MooCodeExpr *cur;

	env->set("this", m_this);
	for (i = 0, cur = params; cur && i <= m_args->last(); i++, cur = cur->next()) {
		id = cur->get_identifier();
		if (!strcmp(id, "&all"))
			return(0);
		else
			env->set(id, m_args->get(i));
	}
	if (cur || i <= m_args->last())
		throw moo_args_mismatched;
	return(0);
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

