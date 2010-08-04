/*
 * Object Name:	action.c
 * Description:	Action Base Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

MooObjectType moo_action_obj_type = {
	NULL,
	"action",
	typeid(MooAction).name(),
	(moo_type_create_t) NULL
};

MooAction::MooAction(moo_action_t func)
{
	m_func = func;
}

int MooAction::do_action(MooThing *thing, MooArgs *args)
{

	return(0);
}

