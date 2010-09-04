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

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>

static int channel_init(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_join(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_leave(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_announce(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_say(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_channel_actions(MooBuiltinHash *actions)
{
	// TODO should this specify various defaults of these actions?
	// TODO should there be a function that creates either a special root object with props and actions set, or
	//	possibly a function which creates an equally appropriate subobject which inherits from a core object?
	actions->set("channel_init", new MooBuiltin(channel_init));
	actions->set("channel_join", new MooBuiltin(channel_join));
	actions->set("channel_leave", new MooBuiltin(channel_leave));
	actions->set("channel_announce", new MooBuiltin(channel_announce));
	actions->set("channel_say", new MooBuiltin(channel_say));
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
	// TODO send message to all users
	// TODO add user to list
	return(0);
}

static int channel_leave(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO send message to all users
	// TODO remove user from list
	return(0);
}

static int channel_announce(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	MooObjectArray *users;

	// TODO should this function actually take TNT_SAY as an argument so that it can also be used for TNT_EMOTE?
	if (!(users = (MooObjectArray *) args->m_this->get_property("users", &moo_array_obj_type)))
		return(-1);
	// TODO should there be an easier way to traverse a list of things?
	for (int i = 0; i < users->last(); i++) {
		if ((cur = users->get_thing(i)))
			cur->notify(TNT_SAY, args->m_this, args->m_user, args->m_text);
	}
	return(0);
}

static int channel_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	if (*args->m_text == '\0')
		return(-1);
	return(channel_announce(action, thing, args));
}



