/*
 * Name:	moocode.c
 * Description:	MooCode Actions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>

#include <sdm/code/code.h>
#include <sdm/actions/action.h>
#include <sdm/actions/builtin/builtin.h>


static int moocode_eval(MooAction *action, MooThing *thing, MooObjectHash *env, MooArgs *args)
{
	MooCodeFrame frame;

	return(frame.eval(args->m_args->get_string(0), args));
}

int moo_load_moocode_actions(MooBuiltinHash *actions)
{
	actions->set("moocode_eval", new MooBuiltin(moocode_eval));
	return(0);
}

