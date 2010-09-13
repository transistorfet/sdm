/*
 * Name:	channel.cpp
 * Description:	Channel Actions
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
#include <sdm/objs/thingref.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>

static int channel_init(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_join(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_leave(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_quit(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_say(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_emote(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_names(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_evaluate(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_join(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_leave(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_quit(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_say(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_emote(MooAction *action, MooThing *thing, MooArgs *args);
static int realm_evaluate(MooAction *action, MooThing *thing, MooArgs *args);

static int cryolocker_store(MooThing *thing);
static int cryolocker_revive(MooThing *thing);

int moo_load_channel_actions(MooBuiltinHash *actions)
{
	// TODO should this specify various defaults of these actions?
	// TODO should there be a function that creates either a special root object with props and actions set, or
	//	possibly a function which creates an equally appropriate subobject which inherits from a core object?
	actions->set("channel_init", new MooBuiltin(channel_init));
	actions->set("channel_join", new MooBuiltin(channel_join));
	actions->set("channel_leave", new MooBuiltin(channel_leave));
	actions->set("channel_quit", new MooBuiltin(channel_quit));
	actions->set("channel_say", new MooBuiltin(channel_say));
	actions->set("channel_emote", new MooBuiltin(channel_emote));
	actions->set("channel_names", new MooBuiltin(channel_names));
	actions->set("channel_evaluate", new MooBuiltin(channel_evaluate));
	actions->set("realm_join", new MooBuiltin(realm_join));
	actions->set("realm_leave", new MooBuiltin(realm_leave));
	actions->set("realm_quit", new MooBuiltin(realm_quit));
	actions->set("realm_say", new MooBuiltin(realm_say));
	actions->set("realm_emote", new MooBuiltin(realm_emote));
	actions->set("realm_evaluate", new MooBuiltin(realm_evaluate));
	return(0);
}

static int channel_init(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO set channel name
	// TODO create list of users object
	// TODO add object to list of channels (which is used for distributing quit messages)???  (or possibly you
	//	make it "inside" the channels list object)
	return(0);
}

static int channel_join(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	MooObjectArray *users;

	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	/// If the user is already in the channel, then just return
	for (int i = 0; i <= users->last(); i++) {
		if (users->get_thing(i) == args->m_user)
			return(0);
	}
	// TODO should this maybe use an object sent as an arg so that non-users can join the channel?
	users->push(new MooThingRef(args->m_user->id()));
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_JOIN, args->m_user, args->m_this, NULL);
	}
	return(0);
}

static int channel_leave(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	MooObject *ref;
	MooObjectArray *users;

	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if (users->get_thing(i) == args->m_user) {
			for (int j = 0; j < users->last() + 1; j++) {
				if ((cur = users->get_thing(j)))
					cur->notify(TNT_LEAVE, args->m_user, args->m_this, NULL);
			}
			ref = users->splice(i);
			delete ref;
			return(0);
		}
	}
	return(0);
}

static int channel_quit(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooObject *ref;
	MooObjectArray *users;

	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if (users->get_thing(i) == args->m_user) {
			ref = users->splice(i);
			delete ref;
			return(0);
		}
	}
	return(0);
}

static int channel_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	const char *text;
	MooObjectArray *users;

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_SAY, args->m_user, args->m_this, text);
	}
	return(0);
}

static int channel_emote(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	const char *text;
	MooObjectArray *users;

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_EMOTE, args->m_user, args->m_this, text);
	}
	return(0);
}

static int channel_names(MooAction *action, MooThing *thing, MooArgs *args)
{
	int j = 0;
	MooThing *cur;
	MooObjectArray *users;
	char buffer[LARGE_STRING_SIZE];

	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)) && cur->is_a(&moo_user_obj_type)) {
			strncpy(&buffer[j], ((MooUser *) cur)->name(), LARGE_STRING_SIZE - j);
			j += strlen(((MooUser *) cur)->name());
			if (j >= LARGE_STRING_SIZE)
				break;
			buffer[j++] = ' ';
		}
	}
	if (j > 0)
		buffer[--j] = '\0';
	args->m_result = new MooString(buffer);
	return(0);
}

static int channel_evaluate(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO you could also just print a message like "Commands are not supported in this channel"
	args->m_user->notify(TNT_STATUS, NULL, NULL, "Commands are not supported in this channel.");

	// TODO should you check to make sure this doesn't loop?
	//return(args->m_user->command(args->m_text));
	return(0);
}


///// Realm Actions /////

static int realm_join(MooAction *action, MooThing *thing, MooArgs *args)
{
	if (channel_join(action, thing, args) < 0)
		return(-1);
	try {
		cryolocker_revive(args->m_user);
	}
	catch (...) {
		moo_status("USER: Error reviving user from cryolocker, %s", args->m_user->name());
	}

	// TODO do a look action
	return(0);
}

static int realm_leave(MooAction *action, MooThing *thing, MooArgs *args)
{
	if (channel_leave(action, thing, args) < 0)
		return(-1);

	try {
		cryolocker_store(args->m_user);
	}
	catch (...) {
		moo_status("USER: Error storing user in cryolocker, %s", args->m_user->name());
	}
	return(0);
}

static int realm_quit(MooAction *action, MooThing *thing, MooArgs *args)
{
	if (channel_quit(action, thing, args) < 0)
		return(-1);

	try {
		cryolocker_store(args->m_user);
	}
	catch (...) {
		moo_status("USER: Error storing user in cryolocker, %s", args->m_user->name());
	}
	return(0);
}

static int realm_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *location = args->m_user->location();
	if (location)
		return(location->do_action("say", args));
	return(0);
}

static int realm_emote(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *location = args->m_user->location();
	if (location)
		return(location->do_action("emote", args));
	return(0);
}

static int realm_evaluate(MooAction *action, MooThing *thing, MooArgs *args)
{
	const char *text;

	text = args->m_args->get_string(0);
	// TODO should you check to make sure this doesn't loop? (ie. the command isn't evaluate)
	return(args->m_user->command(args->m_user, args->m_channel, text));
	return(0);
}


static int cryolocker_store(MooThing *thing)
{
	MooThing *cryolocker;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	// TODO could this be dangerous if you had to create a new cryolocker object, and now it will move the thing even though
	//	it was already in the cryolocker, causing last_location to be erroneusly overwritten
	if (thing->location() != cryolocker) {
		thing->set_property("last_location", thing->location()->id());
		// TODO call the action needed to notify that the object is leaving (so everyone in the room sees "Soandso leaves in a
		//	puff of smoke" or something like that

		// TODO how should the quit message thing work?  where will it come from (property?)
		// TODO this is totally wrong, this should be sent via some other means
		//if (this->m_location)
		//	this->m_location->notify_all(TNT_QUIT, this, NULL, "disappears in a puff of smoke.");
		// TODO what happens if this isn't allow? like if you are fighting
		thing->moveto(cryolocker, NULL);
	}
	return(0);
}

static int cryolocker_revive(MooThing *thing)
{
	MooThing *cryolocker, *location;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	if (thing->location() == cryolocker) {
		if (!(location = thing->get_thing_property("last_location")))
			return(-1);
		// TODO how the fuck does the 'by' param work?
		if (thing->moveto(location, NULL)) {
			if ((location = MooThing::reference(MOO_START_ROOM)))
				thing->moveto(location, NULL);
		}
	}
	return(0);
}


