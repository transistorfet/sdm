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

#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/mud.h>
#include <sdm/actions/builtin/builtin.h>

static int room_init(MooAction *action, MooThing *thing, MooArgs *args);
static int room_say(MooAction *action, MooThing *thing, MooArgs *args);
static int room_emote(MooAction *action, MooThing *thing, MooArgs *args);
static int room_whisper(MooAction *action, MooThing *thing, MooArgs *args);
static int room_look(MooAction *action, MooThing *thing, MooArgs *args);
static int room_direction(MooAction *action, MooThing *thing, MooArgs *args);
static int room_do_enter(MooAction *action, MooThing *thing, MooArgs *args);
static int room_do_leave(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_room_actions(MooBuiltinHash *actions)
{
	actions->set("room_init", new MooBuiltin(room_init));
	actions->set("room_say", new MooBuiltin(room_say));
	actions->set("room_emote", new MooBuiltin(room_emote));
	actions->set("room_whisper", new MooBuiltin(room_whisper));
	actions->set("room_look", new MooBuiltin(room_look));
	actions->set("room_direction", new MooBuiltin(room_direction));
	actions->set("room_do_enter", new MooBuiltin(room_do_enter));
	actions->set("room_do_leave", new MooBuiltin(room_do_leave));
	return(0);
}

static int room_init(MooAction *action, MooThing *thing, MooArgs *args)
{

	return(0);
}

static int room_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	//MooThing *cur;
	const char *text;

	// TODO Check if room silence bit is set for this room
	//if (CAN_SPEAK(args->m_this, args->m_user))		// TODO is user correct here??

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	// TODO for each user/npc/whatever
	//		if ignoring bit set, don't send message
	//		perhaps if the person trying to speak is special (immortal) then speak despite ignore bit
	args->m_user->location()->notify_all(TNT_SAY, args->m_user, args->m_channel, text);

	// TODO check for room/object/mobile triggers based on speech

	return(0);
}

static int room_emote(MooAction *action, MooThing *thing, MooArgs *args)
{
	//MooThing *cur;
	const char *text;

	// TODO check room_say for modification suggestions
	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	args->m_user->location()->notify_all(TNT_EMOTE, args->m_user, args->m_channel, text);
	return(0);
}

static int room_whisper(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO check that the user is the room
	// TODO check if the user is yourself (Smaug: "You have a nice little chat with yourself.\n\r")
	// TODO switched? link-dead? afk?
	// TODO check if user is deaf (whispers disallowed) or silenced (forced)
	// TODO check if user is ignoring you
	// TODO send messages to the sender and reciever
	// TODO send message to all other users unless room is silenced ("$n whispers something to $N.")
	// TODO check for room/object/mobile triggers based on speech
	return(0);
}

static int room_look(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *object;

	// TODO check if room is dark, etc?
	// TODO proximity check on object??? so you can't look at public things far away
	if (!(object = args->m_args->get_thing(0)))
		object = args->m_this;
	// TODO notify all the people in the room that you are looking at something (should this be here or in look_self?)
	object->do_action(args->m_user, args->m_channel, "look_self");
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



