/*
 * Name:	alias.c
 * Description:	Command Aliases
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/user.h>

#include <sdm/actions/action.h>
#include <sdm/actions/alias.h>

MooObjectType moo_alias_obj_type = {
	&moo_action_obj_type,
	"alias",
	typeid(MooAlias).name(),
	(moo_type_create_t) moo_alias_create
};

MooObject *moo_alias_create(void)
{
	return(new MooAlias());
}

MooAlias::MooAlias(MooThing* thing, const char *command) : MooAction(thing)
{
	m_command = command ? new std::string(command) : NULL;
}

MooAlias::~MooAlias()
{
	if (m_command)
		delete m_command;
}

int MooAlias::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "alias")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		if (m_command)
			delete m_command;
		m_command = new std::string(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooAlias::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_string_entry("alias", m_command->c_str());
	return(0);
}

int MooAlias::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	char buffer[STRING_SIZE];

	// TODO use frame to do the actual evaluation

	// TODO how do you add the args to the environment? using make_env or just as 'args' or what?  Should it/will it mask an
	//	args declaration for the previous call?  Should it extend the environment so that it doesn't get modified
	MooThing::format(buffer, STRING_SIZE, env, m_command->c_str());
	return(args->m_user->command(args->m_user, args->m_channel, buffer));
}


