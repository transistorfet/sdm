/*
 * Name:	item.cpp
 * Description:	Item Actions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>


static int item_get(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	// TODO this just gets one arg for now because parsing is broken
	if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "You don't see that here.\n");
		return(0);
	}
	if (args->obj->location == args->caller) {
		sdm_notify(args->caller, args, "You already have that.\n");
		return(0);
	}
	sdm_moveto(args->caller, args->obj, args->caller, NULL);
*/
	return(0);
}

static int item_drop(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	int i = 0;

	if (!args->obj && ((*args->text == '\0')
	    || !(args->obj = sdm_interpreter_get_thing(args->caller, args->text, &i))
	    || (args->obj->location != args->caller))) {
		sdm_notify(args->caller, args, "You aren't carrying that.\n");
		return(-1);
	}
	sdm_moveto(args->caller, args->obj, args->caller->location, NULL);
*/
	return(0);
}

int moo_load_item_actions(MooBuiltinHash *actions)
{
	actions->set("item_get", new MooBuiltin(item_get));
	actions->set("item_drop", new MooBuiltin(item_drop));
	return(0);
}


