/*
 * Name:	basics.c
 * Description:	Basic Actions
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
#include <sdm/things/world.h>

#include <sdm/actions/action.h>
#include <sdm/actions/builtin/builtin.h>

static int basics_tell(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_tell_view(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_examine(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_go(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_basic_actions(MooBuiltinHash *actions)
{
	actions->set("basics_tell", new MooBuiltin(basics_tell));
	actions->set("basics_tell_view", new MooBuiltin(basics_tell_view));
	actions->set("basics_examine", new MooBuiltin(basics_examine));
	actions->set("basics_go", new MooBuiltin(basics_go));
	return(0);
}


static int basics_tell(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO i have no idea how this would work.  a direct user:print() would be weird.  How would you
	//	determine the channel and thing objects and stuff.  What is actually needed it terms of
	//	permissions checks, and possibly uses for this function?
/*
	if (!sdm_is_user(thing) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args->text));
*/
	// TODO NULL should be the sender, and is m_text right?
	args->m_user->print(args, args->m_text);
	return(0);
}

static int basics_tell_view(MooAction *action, MooThing *thing, MooArgs *args)
{
	args->m_user->print(args, "<brightblue>You see $this.title here.");
	return(0);
}

static int basics_examine(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;

	args->m_user->print(args, "<brightyellow>$this.title");
	args->m_user->print(args, "<brightgreen>$this.description");
	for (cur = args->m_this->contents(); cur; cur = cur->next()) {
		if (cur == args->m_caller || cur == args->m_user)
			continue;
		cur->do_action("tell_view", args->m_user, NULL, NULL);
	}
	return(0);
}

static int basics_go(MooAction *action, MooThing *thing, MooArgs *args)
{
/*
	double num;
	struct sdm_thing *location;

	// TODO check that the given object can use this exit
	if (((num = sdm_get_number_property(thing, "target")) <= 0)
	    || !(location = sdm_thing_lookup_id((sdm_id_t) num)))
		return(-1);
	sdm_moveto(args->caller, args->caller, location, thing);
*/
	return(0);
}

