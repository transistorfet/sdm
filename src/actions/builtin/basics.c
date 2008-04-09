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
#include <sdm/processors/interpreter.h>

#include <sdm/actions/action.h>
#include <sdm/actions/builtin/basics.h>

int sdm_builtin_load_basics(struct sdm_hash *actions)
{
	sdm_hash_add(actions, "builtin_notify", sdm_builtin_action_notify);
	sdm_hash_add(actions, "builtin_announce", sdm_builtin_action_announce);
	sdm_hash_add(actions, "builtin_say", sdm_builtin_action_say);
	sdm_hash_add(actions, "builtin_look", sdm_builtin_action_look);
	sdm_hash_add(actions, "builtin_examine", sdm_builtin_action_examine);
	sdm_hash_add(actions, "builtin_go", sdm_builtin_action_go);
	sdm_hash_add(actions, "builtin_direction", sdm_builtin_action_direction);

	sdm_hash_add(actions, "builtin_inventory", sdm_builtin_action_inventory);
	sdm_hash_add(actions, "builtin_get", sdm_builtin_action_get);
	sdm_hash_add(actions, "builtin_drop", sdm_builtin_action_drop);

	sdm_hash_add(actions, "builtin_room_do_enter", sdm_builtin_action_room_do_enter);
	sdm_hash_add(actions, "builtin_room_do_leave", sdm_builtin_action_room_do_leave);
	return(0);
}


/**
 * Send a message to a user's output device.
 *	caller:		object that is notifying the user something
 *	thing:		the user to notify
 *	target:		not used
 *	args:		the text to output
 */
int sdm_builtin_action_notify(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (!sdm_is_user(thing) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args->text));
}

/**
 * Send a message to all children of a container.
 *	caller:		object that is announcing
 *	thing:		the container
 *	target:		not used
 *	args:		the text to output
 */
int sdm_builtin_action_announce(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *cur;

	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur != args->caller)
			sdm_notify(cur, args, "%s", args->text);
	}
	return(0);
}

/**
 * Send a message to all everyone in the same room as the thing.
 *	caller:		object that is saying something
 *	thing:		the room in which something is being said
 *	target:		not used
 *	args:		the text to output
 */
int sdm_builtin_action_say(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (*args->text == '\0')
		return(-1);
	sdm_notify(args->caller, args, "You say \"$text\"\n");
	sdm_announce(thing, args, "\n$caller.name says \"$text\"\n");
	return(0);
}

/**
 * Perform the 'examine' action on the target object or on the object itself if no target is given.
 *	caller:		object that is looking
 *	thing:		the room in which the caller is located
 *	target:		the object being looked at
 *	args:		the name of the object being looked at if target is NULL, unused otherwise
 */
int sdm_builtin_action_look(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (*args->text == '\0')
		args->obj = thing;
	else if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "You don't see that here.\n");
		return(0);
	}
	sdm_do_nil_action(args->obj, args->caller, "look_self");
	return(0);
}

/**
 * Print a description of a thing and list it's contents if it's a container
 *	caller:		object that is examining
 *	thing:		the object being examined
 *	target:		not used
 *	args:		not used
 */
int sdm_builtin_action_examine(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *cur;

	sdm_notify(args->caller, args, "<brightyellow>$thing.name</brightyellow>\n");
	sdm_notify(args->caller, args, "<brightgreen>$thing.description</brightgreen>\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur == args->caller)
			continue;
		sdm_do_nil_action(cur, args->caller, "tell_view");
	}
	return(0);
}

/**
 * Move the caller to the target of thing.
 *	caller:		object to be moved
 *	thing:		the object containing the target of the move
 *	target:		not used
 *	args:		not used
 */
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

/**
 * Move the caller using the exit named as the action command
 *	caller:		object to be moved
 *	thing:		the room the object is in
 */
int sdm_builtin_action_direction(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *exit;

	if (!(exit = sdm_interpreter_find_thing(thing, args->action))) {
		sdm_notify(args->caller, args, "You can't go in that direction.\n");
		return(0);
	}
	sdm_do_nil_action(exit, args->caller, "go");
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
		if (!(str = sdm_get_string_property(cur, "name")))
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


int sdm_builtin_action_room_do_enter(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *exitname;

	if (!args->obj)
		return(0);
	if (sdm_is_mobile(args->obj)) {
		/** A mobile is entering the room */
		if (args->target) {
			/** The mobile is entering through an exit */
			if (!(exitname = sdm_get_string_property(args->target, "name"))
			    || (sdm_get_number_property(args->target, "hidden") > 0))
				exitname = "somewhere";
			sdm_announce(thing, args, "<brightgreen>$obj.name enters from %s.\n", exitname);
		}
		else
			/** The mobile is teleporting here */
			sdm_announce(thing, args, "<brightgreen>$obj.name appears in a shower of sparks.\n");
		sdm_do_nil_action(thing, args->caller, "look_self");
	}
	else {
		/** Assume the object is being dropped by the caller */
		sdm_announce(thing, args, "$caller.name drops $obj.name here.\n");
		sdm_notify(args->caller, args, "You drop $obj.name here.\n");
	}
	return(0);
}

int sdm_builtin_action_room_do_leave(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *exitname;

	if (!args->obj)
		return(0);
	if (sdm_is_mobile(args->obj)) {
		/** A mobile is leaving the room */
		if (args->target) {
			/** The mobile is leaving through an exit */
			if (!(exitname = sdm_get_string_property(args->target, "name"))
			    || (sdm_get_number_property(args->target, "hidden") > 0))
				exitname = "somewhere";
			sdm_announce(thing, args, "<brightgreen>$obj.name leaves %s.\n", exitname);
		}
		else
			/** The mobile is teleporting away */
			sdm_announce(thing, args, "<brightgreen>$obj.name vanishes in a puff of smoke.\n");
	}
	else {
		/** Assume the object is being picked up by the caller */
		sdm_announce(thing, args, "$caller.name gets $obj.name.\n");
		sdm_notify(args->caller, args, "You get $obj.name.\n");
	}
	return(0);
}



