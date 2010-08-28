/*
 * Name:	user.cpp
 * Description:	User Actions
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

static int user_init(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_user_actions(MooBuiltinHash *actions)
{
	actions->set("user_init", new MooBuiltin(user_init));
	return(0);
}


static int user_init(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO set any properties or whatever needs to be found on any user thing
	return(0);
}

