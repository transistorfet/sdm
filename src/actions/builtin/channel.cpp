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
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>

static int channel_join(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_leave(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_announce(MooAction *action, MooThing *thing, MooArgs *args);
static int channel_say(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_channel_actions(MooBuiltinHash *actions)
{
	// TODO should this specify various defaults of these actions?
	// TODO should there be a function that creates either a special root object with props and actions set, or
	//	possibly a function which creates an equally appropriate subobject which inherits from a core object?
	actions->set("channel_join", new MooBuiltin(channel_join));
	actions->set("channel_leave", new MooBuiltin(channel_leave));
	actions->set("channel_announce", new MooBuiltin(channel_announce));
	actions->set("channel_say", new MooBuiltin(channel_say));
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

	for (cur = args->m_this->contents(); cur; cur = cur->next()) {
		// TODO should this actually omit the user, or due to the non-formatted text dispatch, should it be ok?
		if (cur != args->m_user) {
			//sdm_notify(cur, args, "%s", args->text);
			//cur->talk(args->m_this, args->m_user, "%s", args->m_text);
			// TODO ^^ what the fuck?  Would MooThing have talk? Or do you need to do this as an action...
			//cur->do_action("notify", "say", args->m_this, args->m_user, "%s", args->m_text);
			// TODO ^^ this is even more bizzare.  How will these params map to MooArgs??
		}
	}
	return(0);
}

static int channel_say(MooAction *action, MooThing *thing, MooArgs *args)
{
	if (*args->m_text == '\0')
		return(-1);
	//user->printf(args, "You say \"$text\"");
	//channel_announce(thing, args, "$user.name says \"$text\"");
	//args->m_this->do_action("announce", args->m_user, ???, args->m_text);
	return(0);
}



