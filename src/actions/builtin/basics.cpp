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

static int basics_register(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_print(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_print_view(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_look_self(MooAction *action, MooThing *thing, MooArgs *args);
static int basics_go(MooAction *action, MooThing *thing, MooArgs *args);

int moo_load_basic_actions(MooBuiltinHash *actions)
{
	actions->set("basics_register", new MooBuiltin(basics_register));
	actions->set("basics_print", new MooBuiltin(basics_print));
	actions->set("basics_print_view", new MooBuiltin(basics_print_view));
	actions->set("basics_look_self", new MooBuiltin(basics_look_self));
	actions->set("basics_go", new MooBuiltin(basics_go));
	return(0);
}

static int basics_register(MooAction *action, MooThing *thing, MooArgs *args)
{
	// TODO register new user
	return(0);
}

static int basics_print(MooAction *action, MooThing *thing, MooArgs *args)
{
	args->m_user->print(args, args->m_text);
	return(0);
}

static int basics_print_view(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooString *name;
	// TODO this should only print this if $this.title exists (use $this.name if it's not there)
	if (!(name = (MooString *) args->m_this->get_property("title", &moo_string_obj_type)))
		name = (MooString *) args->m_this->get_property("name", &moo_string_obj_type);
	args->m_user->printf(args, "<brightblue>You see %s here.", name->m_str);
	return(0);
}

static int basics_look_self(MooAction *action, MooThing *thing, MooArgs *args)
{
	MooThing *cur;
	MooString *str;

	// TODO this should somehow check if title and description exist and don't print the message if it doesn't exist
	args->m_user->print(args, "<brightyellow>$this.title");
	args->m_user->print(args, "<brightgreen>$this.description");
	/// Print the views of all objects contained in the target
	for (cur = args->m_this->contents(); cur; cur = cur->next()) {
		if (cur == args->m_caller || cur == args->m_user)
			continue;
		cur->do_action("print_view", args->m_user, NULL, NULL);
	}

	// TODO you should print a different set of messages if the look failed
	// TODO you can transfer a lot of this "message response" code to a special function which takes a list of
	//	responses and prints them under the various conditions

	/// Print a message to all objects in the room that someone is looking at something
	for (cur = args->m_user->location()->contents(); cur; cur = cur->next()) {
		if (cur == args->m_user)	/// Don't print a message to the user
			continue;
		if (args->m_user == args->m_this)
			//print "checks out self" message to cur
			str = (MooString *) args->m_this->get_property("self_looked_at", &moo_string_obj_type);
		else if (cur == args->m_this)
			//print "victimized message"
			//(if someone looks at you, the message should come from whatever is looking at you
			str = (MooString *) args->m_user->get_property("look_at", &moo_string_obj_type);
		else
			//print "3rd party" message
			//("You see $user.name looking around the room confused")
			//("$user.name examines a $this.name")
			//This comes from the object being looked at
			str = (MooString *) args->m_this->get_property("looked_at", &moo_string_obj_type);
		if (str)
			cur->print(args, str->m_str);
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

