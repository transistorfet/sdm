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

MooObjectType moo_builtin_obj_type = {
	&moo_action_obj_type,
	"builtin",
	typeid(MooBuiltin).name(),
	(moo_type_create_t) moo_builtin_create
};

//static struct sdm_hash *builtin_actions = NULL;

int init_builtin(void)
{
	if (moo_object_register_type(&moo_builtin_obj_type) < 0)
		return(-1);
/*
	if (builtin_actions)
		return(1);
	if (!(builtin_actions = create_sdm_hash(0, -1, NULL)))
		return(-1);
	sdm_builtin_load_basics(builtin_actions);
	sdm_builtin_load_builder(builtin_actions);
	sdm_builtin_load_room_actions(builtin_actions);
*/
	return(0);
}

void release_builtin(void)
{
/*
	if (!builtin_actions)
		return;
	destroy_sdm_hash(builtin_actions);
	builtin_actions = NULL;
*/
	moo_object_deregister_type(&moo_builtin_obj_type);
}

MooObject *moo_builtin_create(void)
{
	return(new MooBuiltin());
}

MooBuiltin::MooBuiltin(moo_action_t func)
{
	m_func = func;
}

int MooBuiltin::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

	if (data->read_string(buffer, STRING_SIZE) < 0)
		return(-1);
	//if (!(m_entry = builtin_actions->find_entry(buffer)))
	//	return(-1);
	//m_func = (sdm_action_t) m_entry->m_data;
	return(MOO_HANDLED_ALL);
}

int MooBuiltin::write_data(MooDataFile *data)
{
	//data->write_string(m_entry->name);
	return(0);
}

int MooBuiltin::do_action(MooThing *thing, MooArgs *args)
{
	return(m_func(this, thing, args));
}



/**
 * tell function: will call a function user->tell directly

 */


