/*
 * Name:	channel.cpp
 * Description:	Channel Actions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/objs/thingref.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>

static int cryolocker_store(MooThing *user, MooThing *channel);
static int cryolocker_revive(MooThing *user, MooThing *channel);

static int channel_init(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO set channel name
	// TODO create list of users object
	// TODO add object to list of channels (which is used for distributing quit messages)???  (or possibly you
	//	make it "inside" the channels list object)
	return(0);
}

static int channel_join(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;
	MooObjectArray *users;

	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
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
			cur->notify(TNT_JOIN, args->m_user, (MooThing *) args->m_this, NULL);
	}
	return(0);
}

static int channel_leave(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;
	MooObject *ref;
	MooObjectArray *users;

	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if (users->get_thing(i) == args->m_user) {
			for (int j = 0; j < users->last() + 1; j++) {
				if ((cur = users->get_thing(j)))
					cur->notify(TNT_LEAVE, args->m_user, (MooThing *) args->m_this, NULL);
			}
			ref = users->splice(i);
			MOO_DECREF(ref);
			return(0);
		}
	}
	return(0);
}

static int channel_quit(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *ref;
	MooObjectArray *users;

	// TODO how will other users receive the quit message
	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if (users->get_thing(i) == args->m_user) {
			ref = users->splice(i);
			MOO_DECREF(ref);
			return(0);
		}
	}
	return(0);
}

static int channel_say(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;
	const char *text;
	MooObjectArray *users;

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_SAY, args->m_user, (MooThing *) args->m_this, text);
	}
	return(0);
}

static int channel_emote(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;
	const char *text;
	MooObjectArray *users;

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_EMOTE, args->m_user, (MooThing *) args->m_this, text);
	}
	return(0);
}

static int channel_names(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int j = 0;
	MooThing *cur;
	const char *name;
	MooObjectArray *users;
	char buffer[LARGE_STRING_SIZE];

	if (!(users = dynamic_cast<MooObjectArray *>(args->m_this->resolve_property("users"))))
		return(-1);
	for (int i = 0; i <= users->last(); i++) {
		if ((cur = users->get_thing(i))) {
			name = cur->name();
			strncpy(&buffer[j], name, LARGE_STRING_SIZE - j);
			j += strlen(name);
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

static int channel_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO you could also just print a message like "Commands are not supported in this channel"
	args->m_user->notify(TNT_STATUS, NULL, NULL, "Commands are not supported in this channel.");

	// TODO should you check to make sure this doesn't loop?
	//return(args->m_user->command(args->m_text));
	return(0);
}


///// Realm Actions /////

// TODO move these all to a separate file (since we probably wont need to call any channel functions directly)
static int realm_join(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO should the realm channel bother maintaining a user list?
	if (channel_join(frame, env, args) < 0)
		return(-1);
	args->m_user->notify(TNT_JOIN, args->m_user, (MooThing *) args->m_this, NULL);
	try {
		cryolocker_revive(args->m_user, args->m_channel);
	}
	catch (...) {
		moo_status("USER: Error reviving user from cryolocker, %s", args->m_user->name());
	}

	// TODO do a look action
	return(0);
}

static int realm_leave(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (channel_leave(frame, env, args) < 0)
		return(-1);
	args->m_user->notify(TNT_LEAVE, args->m_user, (MooThing *) args->m_this, NULL);

	try {
		cryolocker_store(args->m_user, args->m_channel);
	}
	catch (...) {
		moo_status("USER: Error storing user in cryolocker, %s", args->m_user->name());
	}
	return(0);
}

static int realm_quit(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (channel_quit(frame, env, args) < 0)
		return(-1);

	try {
		cryolocker_store(args->m_user, args->m_channel);
	}
	catch (...) {
		moo_status("USER: Error storing user in cryolocker, %s", args->m_user->name());
	}
	return(0);
}

static int realm_say(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *func;
	MooThing *location;

	if ((location = args->m_user->location()) && (func = location->resolve_method("say")))
		frame->push_call(env, func, args);
	return(0);
}

static int realm_emote(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *func;
	MooThing *location;

	if ((location = args->m_user->location()) && (func = location->resolve_method("emote")))
		frame->push_call(env, func, args);
	return(0);
}

static int realm_names(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *name;

	if (!(name = args->m_user->name()))
		return(-1);
	args->m_result = new MooString(name);
	return(0);
}

#define MAX_WORDS	256

static int realm_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int i = 0, j = 0;
	const char *text;
	MooObject *method;
	char *words[MAX_WORDS];
	char buffer[LARGE_STRING_SIZE];
	MooArgs *newargs;
	MooObjectHash *newenv;

	text = args->m_args->get_string(0);
	strncpy(buffer, text, LARGE_STRING_SIZE);
	buffer[LARGE_STRING_SIZE] = '\0';
	while (parser_is_whitespace(buffer[i]))
		i++;
	words[0] = &buffer[i];
	for (; buffer[i] != '\0'; i++) {
		if (parser_is_whitespace(buffer[i])) {
			buffer[i++] = '\0';
			while (parser_is_whitespace(buffer[i]))
				i++;
			words[++j] = &buffer[i];
		}
		else if (buffer[i] == '\"') {
			// TODO implement quotes...
		}
	}

	if (!(method = args->m_user->resolve_method(words[0]))) {
		MooObject *location = args->m_user->resolve_property("location");
		if (!(method = location->resolve_method(words[0]))) {
			// TODO try to parse more and search the objects
		}
	}

	// TODO you could push markers onto the stack like here so that if an exception occurs inside of the call we are pushing, then
	//	it will print that it occurred inside of this evaluate function.
	newargs = new MooArgs();
	newenv = new MooObjectHash(env);
	newenv->set("argstr", new MooString(text));
	frame->push_debug("> in realm_evaluate: %s", words[0]);
	frame->push_call(newenv, method, newargs);
	return(0);

/*

	// TODO should you check to make sure this doesn't loop? (ie. the command isn't evaluate)
	//return(args->m_user->command(args->m_user, args->m_channel, text));

	int res;
	MooThing *location;
	MooThingRef *ref = NULL;
	char buffer[STRING_SIZE];

	text = MooArgs::parse_word(buffer, STRING_SIZE, text);
	action = buffer;
	if (text[0] == '\0')
		text = NULL;

	if ((res = args->m_user->call_method(args->m_channel, action, text)) != MOO_ACTION_NOT_FOUND)
		return(res);
	location = args->m_user->location();
	if (location && (res = location->call_method(args->m_channel, action, text)) != MOO_ACTION_NOT_FOUND)
		return(res);
	try {
		MooThing *thing;

		{
			char buffer[STRING_SIZE];
			text = MooArgs::parse_word(buffer, STRING_SIZE, text);
			// TODO this is now invalid because it doesn't check the local area for objects
			ref = new MooThingRef(buffer);
		}
		thing = ref->get();
		if (thing && (res = thing->call_method(args->m_channel, action, text)) != MOO_ACTION_NOT_FOUND)
			throw MooException("You can't do that: %s", action);
	}
	catch (...) { }
	// TODO should this be MOO_DECREF?
	delete ref;
	return(MOO_ACTION_NOT_FOUND);
*/
}


static int cryolocker_store(MooThing *user, MooThing *channel)
{
	MooThing *cryolocker;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	// TODO could this be dangerous if you had to create a new cryolocker object, and now it will move the thing even though
	//	it was already in the cryolocker, causing last_location to be erroneusly overwritten
	if (user->location() != cryolocker) {
		user->resolve_property("last_location", new MooThingRef(user->location()->id()));
		// TODO call the action needed to notify that the object is leaving (so everyone in the room sees "Soandso leaves in a
		//	puff of smoke" or something like that

		// TODO how should the quit message thing work?  where will it come from (property?)
		// TODO this is totally wrong, this should be sent via some other means
		//if (this->m_location)
		//	this->m_location->notify_all(TNT_QUIT, this, NULL, "disappears in a puff of smoke.");
		// TODO what happens if this isn't allow? like if you are fighting
		user->moveto(user, channel, cryolocker);
	}
	return(0);
}

static int cryolocker_revive(MooThing *user, MooThing *channel)
{
	MooObject *obj;
	MooThing *cryolocker, *location;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	if (user->location() == cryolocker) {
		if (!(obj = user->resolve_property("last_location")) || !(location = obj->get_thing()))
			return(-1);
		// TODO how the fuck does the 'by' param work?
		if (user->moveto(user, channel, location)) {
			if ((location = MooThing::reference(MOO_START_ROOM)))
				user->moveto(user, channel, location);
		}
	}
	return(0);
}

int moo_load_channel_actions(MooObjectHash *env)
{
	// TODO should this specify various defaults of these actions?
	env->set("channel_init", new MooCodeFunc(channel_init));
	env->set("channel_join", new MooCodeFunc(channel_join));
	env->set("channel_leave", new MooCodeFunc(channel_leave));
	env->set("channel_quit", new MooCodeFunc(channel_quit));
	env->set("channel_say", new MooCodeFunc(channel_say));
	env->set("channel_emote", new MooCodeFunc(channel_emote));
	env->set("channel_names", new MooCodeFunc(channel_names));
	env->set("channel_evaluate", new MooCodeFunc(channel_evaluate));
	env->set("realm_join", new MooCodeFunc(realm_join));
	env->set("realm_leave", new MooCodeFunc(realm_leave));
	env->set("realm_quit", new MooCodeFunc(realm_quit));
	env->set("realm_say", new MooCodeFunc(realm_say));
	env->set("realm_emote", new MooCodeFunc(realm_emote));
	env->set("realm_names", new MooCodeFunc(realm_names));
	env->set("realm_evaluate", new MooCodeFunc(realm_evaluate));
	return(0);
}


