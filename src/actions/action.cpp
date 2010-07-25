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
	(moo_type_create_t) moo_action_create
};

int MooAction::MooAction(moo_action_t func)
{
	this->func = func;
}

int MooAction::do_action(MooThing *thing, MooArgs *args)
{

}

