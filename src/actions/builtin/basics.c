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
#include <sdm/things/utils.h>
#include <sdm/processes/interpreter.h>

#include <sdm/actions/action.h>
#include <sdm/actions/builtin/basics.h>

int sdm_builtin_load_basics(struct sdm_hash *actions)
{
	sdm_hash_add(actions, "builtin_notify", sdm_builtin_action_notify);
	sdm_hash_add(actions, "builtin_examine", sdm_builtin_action_examine);
	sdm_hash_add(actions, "builtin_go", sdm_builtin_action_go);

	sdm_hash_add(actions, "builtin_inventory", sdm_builtin_action_inventory);
	sdm_hash_add(actions, "builtin_get", sdm_builtin_action_get);
	sdm_hash_add(actions, "builtin_drop", sdm_builtin_action_drop);
	return(0);
}


int sdm_builtin_action_notify(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (!sdm_is_user(thing) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args->text));
}

int sdm_builtin_action_examine(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *cur;

	sdm_notify(args->caller, args, "<brightyellow>$thing.title</brightyellow>\n");
	sdm_notify(args->caller, args, "<brightgreen>$thing.description</brightgreen>\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur == args->caller)
			continue;
		sdm_do_nil_action(cur, args->caller, "tell_view");
	}
	return(0);
}

int sdm_builtin_action_go(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	double num;
	struct sdm_thing *location;

	// TODO check that the given object can use this exit
	if (((num = sdm_get_number_property(thing, "target")) <= 0)
	    || !(location = sdm_thing_lookup_id((sdm_id_t) num)))
		return(-1);
	sdm_moveto(args->caller, args->caller, location, thing);
	return(0);
}

int sdm_builtin_action_inventory(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
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
	return(0);
}

int sdm_builtin_action_get(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
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
	return(0);
}

int sdm_builtin_action_drop(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	int i = 0;

	if (!args->obj && ((*args->text == '\0')
	    || !(args->obj = sdm_interpreter_get_thing(args->caller, args->text, &i))
	    || (args->obj->location != args->caller))) {
		sdm_notify(args->caller, args, "You aren't carrying that.\n");
		return(-1);
	}
	sdm_moveto(args->caller, args->obj, args->caller->location, NULL);
	return(0);
}


