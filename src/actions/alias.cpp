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

MooAlias::MooAlias(const char *name, MooThing* thing, const char *command) : MooAction(name, thing)
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

int MooAlias::do_action(MooThing *thing, MooArgs *args)
{
	char buffer[STRING_SIZE];

	MooThing::expand_str(buffer, STRING_SIZE, args, m_command->c_str());
	return(args->m_user->command(args->m_user, args->m_channel, buffer));
}


