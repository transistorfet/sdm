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

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>

//static int room_say(MooAction *action, MooThing *thing, MooArgs *args);
static int item_inventory(MooAction *action, MooThing *thing, MooArgs *args);
static int item_get(MooAction *action, MooThing *thing, MooArgs *args);
static int item_drop(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_item_actions(MooBuiltinHash *actions)
{
	//actions->set("room_say", new MooBuiltin(room_say));
	actions->set("item_inventory", new MooBuiltin(item_inventory));
	actions->set("item_get", new MooBuiltin(item_get));
	actions->set("item_drop", new MooBuiltin(item_drop));
	return(0);
}


static int item_inventory(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	const char *str;
	struct sdm_thing *cur;

	if (!thing->objects) {
		sdm_notify(args->caller, args, "<brightgreen>You aren't carrying anything.\n");
		return(0);
	}

	sdm_notify(args->caller, args, "<brightgreen>You are carrying:\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (!(str = sdm_get_string_property(cur, "title")))
			continue;
		sdm_notify(args->caller, args, "<brightblue>    %s.\n", str);
	}
*/
	return(0);
}

static int item_get(MooAction *action, MooThing *thing, MooArgs *args)
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

static int item_drop(MooAction *action, MooThing *thing, MooArgs *args)
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


