/*
 * Name:	builtin.c
 * Description:	Builtin Stuff
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
#include <sdm/actions/builtin/basics.h>
#include <sdm/actions/builtin/builtin.h>

#define BUILTIN_LIST_SIZE	32
#define BUILTIN_LIST_BITS	MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL | MOO_HBF_DELETE

MooObjectType moo_builtin_obj_type = {
	&moo_action_obj_type,
	"builtin",
	typeid(MooBuiltin).name(),
	(moo_type_create_t) moo_builtin_create
};

static MooBuiltinHash *builtin_actions = NULL;

int init_builtin(void)
{
	if (moo_object_register_type(&moo_builtin_obj_type) < 0)
		return(-1);
	if (builtin_actions)
		return(1);
	builtin_actions = new MooBuiltinHash(BUILTIN_LIST_SIZE, BUILTIN_LIST_BITS);
	moo_load_basic_actions(builtin_actions);
	return(0);
}

void release_builtin(void)
{
	if (!builtin_actions)
		return;
	delete builtin_actions;
	builtin_actions = NULL;
	moo_object_deregister_type(&moo_builtin_obj_type);
}

MooObject *moo_builtin_create(void)
{
	return(new MooBuiltin());
}

MooBuiltin::MooBuiltin(moo_action_t func)
{
	m_func = func;
	m_master = NULL;
}

int MooBuiltin::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

	if (data->read_string(buffer, STRING_SIZE) < 0)
		return(-1);
	this->set(buffer);
	return(MOO_HANDLED_ALL);
}

int MooBuiltin::write_data(MooDataFile *data)
{
	const char *name;

	// TODO we should do this in a more stable way, since there is a chance it wont be found in the list or master will be NULL
	if (!(name = builtin_actions->key(m_master)))
		return(-1);
	data->write_string(name);
	return(0);
}

int MooBuiltin::do_action(MooThing *thing, MooArgs *args)
{
	return(m_func(this, thing, args));
}

int MooBuiltin::set(const char *name)
{
	MooBuiltin *master;

	if (!(master = builtin_actions->get(name)))
		return(-1);
	m_master = master;
	m_func = master->m_func;
	return(0);
}


