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

MooAlias::MooAlias(const char *command)
{
	if (command)
		m_command = new std::string(command);
	else
		m_command = NULL;
}

MooAlias::~MooAlias()
{
	if (m_command)
		delete m_command;
}

int MooAlias::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

	if (data->read_string(buffer, STRING_SIZE) < 0)
		return(-1);
	if (m_command)
		delete m_command;
	m_command = new std::string(buffer);
	return(MOO_HANDLED_ALL);
}

int MooAlias::write_data(MooDataFile *data)
{
	data->write_string(m_command->c_str());
	return(0);
}

int MooAlias::do_action(MooThing *thing, MooArgs *args)
{
	char buffer[STRING_SIZE];

	// TODO combine and execute
	// args->m_user->command(buffer);
	return(0);
}

