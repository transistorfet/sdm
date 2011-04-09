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


static int room_init(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{

	return(0);
}

static int room_say(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
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

static int room_emote(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
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

static int room_whisper(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
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

static int room_look(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *name;
	MooThing *object;

	// TODO check if room is dark, etc?
	// TODO proximity check on object??? so you can't look at public things far away
	if ((name = args->m_args->get_string(0)))
		object = args->m_user->find(name);
	else
		;//object = args->m_this;
	// TODO notify all the people in the room that you are looking at something (should this be here or in look_self?)
	if (object)
		;//object->do_action(args->m_user, args->m_channel, "look_self");
	else
		args->m_user->notify(TNT_STATUS, args->m_user, args->m_channel, "You don't see that here");
	return(0);
}

static int room_go(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO search this.exits for an exit matching the given name
	// TODO call exit:invoke()
	return(0);
}

static int room_accept(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	args->m_result = new MooInteger(1);
	return(0);
}

static int room_do_enter(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *msg;
	MooThing *cur, *obj;

	if (!(obj = args->get_thing(0)))
		return(-1);
	// TODO how the hell do you tell these things?
	if (1) //obj->is_a_thing(MooThing::reference(MOO_GENERIC_MOBILE)))
		msg = "<blue>$user.name wanders in.";
	else
		msg = "<b><blue>$user.name drops $0.name here.";

	for (cur = args->m_user->location()->contents(); cur; cur = cur->next()) {
		if (cur != args->m_user)
			cur->notify(TNT_STATUS, args, msg);
	}

	if (obj == args->m_user)
		;//args->m_this->do_action(args->m_user, args->m_channel, "look_self");


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

static int room_do_exit(MooAction *action, MooThing *thing, MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;

	for (cur = args->m_user->location()->contents(); cur; cur = cur->next()) {
		if (cur != args->m_user)
			cur->notify(TNT_STATUS, args, "<blue>$user.name runs off in a hurry.");
	}

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

int moo_load_room_actions(MooBuiltinHash *actions)
{
	actions->set("room_init", new MooBuiltin(room_init));
	actions->set("room_say", new MooBuiltin(room_say));
	actions->set("room_emote", new MooBuiltin(room_emote));
	actions->set("room_whisper", new MooBuiltin(room_whisper));
	actions->set("room_look", new MooBuiltin(room_look));
	actions->set("room_go", new MooBuiltin(room_go));
	actions->set("room_accept", new MooBuiltin(room_accept));
	actions->set("room_do_enter", new MooBuiltin(room_do_enter));
	actions->set("room_do_exit", new MooBuiltin(room_do_exit));
	return(0);
}



