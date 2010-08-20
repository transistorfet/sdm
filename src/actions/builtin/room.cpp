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
#include <sdm/actions/builtin/builtin.h>

static int room_announce(MooAction *action, MooThing *thing, MooArgs *args);
static int room_say(MooAction *action, MooThing *thing, MooArgs *args);
static int room_look(MooAction *action, MooThing *thing, MooArgs *args);
static int room_look_self(MooAction *action, MooThing *thing, MooArgs *args);
static int room_direction(MooAction *action, MooThing *thing, MooArgs *args);
static int room_do_enter(MooAction *action, MooThing *thing, MooArgs *args);
static int room_do_leave(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_room_actions(MooBuiltinHash *actions)
{
	actions->set("room_announce", new MooBuiltin(room_announce));
	actions->set("room_say", new MooBuiltin(room_say));
	actions->set("room_look", new MooBuiltin(room_look));
	actions->set("room_look_self", new MooBuiltin(room_look_self));
	actions->set("room_direction", new MooBuiltin(room_direction));
	actions->set("room_do_enter", new MooBuiltin(room_do_enter));
	actions->set("room_do_leave", new MooBuiltin(room_do_leave));
	return(0);
}


static int room_announce(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;

	for (cur = args->m_this->contents(); cur; cur = cur->next()) {
		// TODO should this actually omit the user, or due to the non-formatted text dispatch, should it be ok?
		if (cur != args->m_user && cur->is_a(&moo_user_obj_type)) {
			// TODO for the time being, we are only able to send a notification to MooUsers
			//sdm_notify(cur, args, "%s", args->text);
			//((MooUser *) cur)->notify(cur
		}
	}
	return(0);
}

static int room_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;

	if (*args->m_text == '\0')
		return(-1);
	args->m_user->notify(TNT_SAY, NULL, args->m_user, args->m_text);

	//sdm_announce(thing, args, "$user.name says \"$text\"");
	for (cur = args->m_this->contents(); cur; cur = cur->next()) {
		if (cur != args->m_user && cur->is_a(&moo_user_obj_type))
			((MooUser *) cur)->notify(TNT_SAY, NULL, args->m_user, args->m_text);
	}
	return(0);
}

static int room_look(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	// TODO should this be limited to only looking at objects that are near the user (right now, you
	//	can specify an object number and get the view there without being in that room)
	if (*args->text == '\0')
		args->obj = thing;
	else if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "You don't see that here.\n");
		return(0);
	}
	sdm_do_nil_action(args->obj, args->caller, "look_self");
*/
	return(0);
}

static int room_look_self(MooAction *action, MooThing *thing, MooArgs *args)
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

static int room_direction(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	struct sdm_thing *exit;

	if (!(exit = sdm_interpreter_find_thing(thing, args->action))) {
		sdm_notify(args->caller, args, "You can't go in that direction.\n");
		return(0);
	}
	sdm_do_nil_action(exit, args->caller, "go");
*/
	return(0);
}

static int room_do_enter(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	const char *exitname;

	if (!args->obj)
		return(0);
	if (sdm_is_mobile(args->obj)) {
		// A mobile is entering the room
		if (args->target) {
			// The mobile is entering through an exit
			if (!(exitname = sdm_get_string_property(args->target, "name"))
			    || (sdm_get_number_property(args->target, "hidden") > 0))
				exitname = "somewhere";
			sdm_announce(thing, args, "<brightgreen>$obj.name enters from %s.\n", exitname);
		}
		else
			// The mobile is teleporting here
			sdm_announce(thing, args, "<brightgreen>$obj.name appears in a shower of sparks.\n");
		sdm_do_nil_action(thing, args->caller, "look_self");
	}
	else {
		// Assume the object is being dropped by the caller
		sdm_announce(thing, args, "$caller.name drops $obj.name here.\n");
		sdm_notify(args->caller, args, "You drop $obj.name here.\n");
	}
*/
	return(0);
}

static int room_do_leave(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	const char *exitname;

	if (!args->obj)
		return(0);
	if (sdm_is_mobile(args->obj)) {
		// A mobile is leaving the room
		if (args->target) {
			// The mobile is leaving through an exit
			if (!(exitname = sdm_get_string_property(args->target, "name"))
			    || (sdm_get_number_property(args->target, "hidden") > 0))
				exitname = "somewhere";
			sdm_announce(thing, args, "<brightgreen>$obj.name leaves %s.\n", exitname);
		}
		else
			// The mobile is teleporting away
			sdm_announce(thing, args, "<brightgreen>$obj.name vanishes in a puff of smoke.\n");
	}
	else {
		// Assume the object is being picked up by the caller
		sdm_announce(thing, args, "$caller.name gets $obj.name.\n");
		sdm_notify(args->caller, args, "You get $obj.name.\n");
	}
*/
	return(0);
}



