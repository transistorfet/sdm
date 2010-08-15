/*
 * Name:	basics.c
 * Description:	Basic Actions
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

#include <sdm/actions/action.h>
#include <sdm/actions/builtin/builtin.h>
#include <sdm/actions/builtin/basics.h>

int moo_load_basic_actions(MooBuiltinHash *actions)
{
	actions->set("builtin_notify", new MooBuiltin(moo_basics_notify));
	actions->set("builtin_examine", new MooBuiltin(moo_basics_examine));
	actions->set("builtin_go", new MooBuiltin(moo_basics_go));

	actions->set("builtin_inventory", new MooBuiltin(moo_basics_inventory));
	actions->set("builtin_get", new MooBuiltin(moo_basics_get));
	actions->set("builtin_drop", new MooBuiltin(moo_basics_drop));
	return(0);
}


int moo_basics_tell(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO i have no idea how this would work.  a direct user:print() would be weird.  How would you
	//	determine the channel and thing objects and stuff.  What is actually needed it terms of
	//	permissions checks, and possibly uses for this function?
/*
	if (!sdm_is_user(thing) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args->text));
*/
}

int moo_basics_examine(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	struct sdm_thing *cur;

	sdm_notify(args->caller, args, "<brightyellow>$thing.title</brightyellow>\n");
	sdm_notify(args->caller, args, "<brightgreen>$thing.description</brightgreen>\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur == args->caller)
			continue;
		sdm_do_nil_action(cur, args->caller, "tell_view");
	}
*/
	return(0);
}

int moo_basics_go(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	double num;
	struct sdm_thing *location;

	// TODO check that the given object can use this exit
	if (((num = sdm_get_number_property(thing, "target")) <= 0)
	    || !(location = sdm_thing_lookup_id((sdm_id_t) num)))
		return(-1);
	sdm_moveto(args->caller, args->caller, location, thing);
*/
	return(0);
}

int moo_basics_inventory(MooAction *action, MooThing *thing, MooArgs *args)
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

int moo_basics_get(MooAction *action, MooThing *thing, MooArgs *args)
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

int moo_basics_drop(MooAction *action, MooThing *thing, MooArgs *args)
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


