/*
 * Name:	room.c
 * Description:	Room Actions
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

static int room_announce(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_say(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_look(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_look_self(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_direction(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_do_enter(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int room_do_leave(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

int sdm_builtin_load_room_actions(struct sdm_hash *actions)
{
	sdm_hash_add(actions, "room_announce", room_announce);
	sdm_hash_add(actions, "room_say", room_say);
	sdm_hash_add(actions, "room_look", room_look);
	sdm_hash_add(actions, "room_look_self", room_look_self);
	sdm_hash_add(actions, "room_direction", room_direction);
	sdm_hash_add(actions, "room_do_enter", room_do_enter);
	sdm_hash_add(actions, "room_do_leave", room_do_leave);
	return(0);
}


static int room_announce(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *cur;

	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur != args->caller)
			sdm_notify(cur, args, "%s", args->text);
	}
	return(0);
}

static int room_say(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (*args->text == '\0')
		return(-1);
	sdm_notify(args->caller, args, "You say \"$text\"\n");
	sdm_announce(thing, args, "\n$caller.name says \"$text\"\n");
	return(0);
}

static int room_look(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	// TODO should this be limited to only looking at objects that are near the user (right now, you
	//	can specify an object number and get the view there without being in that room)
	if (*args->text == '\0')
		args->obj = thing;
	else if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "You don't see that here.\n");
		return(0);
	}
	sdm_do_nil_action(args->obj, args->caller, "look_self");
	return(0);
}

static int room_look_self(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
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

static int room_direction(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *exit;

	if (!(exit = sdm_interpreter_find_thing(thing, args->action))) {
		sdm_notify(args->caller, args, "You can't go in that direction.\n");
		return(0);
	}
	sdm_do_nil_action(exit, args->caller, "go");
	return(0);
}

static int room_do_enter(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
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

static int room_do_leave(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
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



