/*
 * Name:	mobile.cpp
 * Description:	Mobile Actions
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

static int mobile_init(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_mobile_actions(MooBuiltinHash *actions)
{
	actions->set("mobile_init", new MooBuiltin(mobile_init));
	return(0);
}


static int mobile_init(MooAction *action, MooThing *thing, MooArgs *args)
{
	//MooThing *cur;

	// TODO Check if room silence bit is set for this room

	//if (*args->m_text == '\0')
	//	return(-1);
	// TODO for each user/npc/whatever
	//		if ignoring bit set, don't send message
	//		perhaps if the person trying to speak is special (immortal) then speak despite ignore bit
	//args->m_user->location()->notify_all(TNT_SAY, args->m_user, args->m_channel, args->m_text);

	// TODO check for room/object/mobile triggers based on speech

	return(0);
}

