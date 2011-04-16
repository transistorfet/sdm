/*
 * Name:	basics.c
 * Description:	Basic Actions
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>


static int basics_register(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO register new user
	return(0);
}

static int basics_print(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO should this be the typical way you reference arguments?
	// TODO i don't really like this, with the type reference...
	args->m_user->notify(TNT_STATUS, args, args->m_args->get_string(0));
	return(0);
}

static int basics_print_view(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *name;

	// TODO make this use different colours for different types of things
	//if ((name = args->m_this->resolve_property("name")))
	//	args->m_user->notify(TNT_STATUS, args, "<b><lightblue>You see %s here.", name);
	return(0);
}

static int basics_look_self(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *cur;
	MooString *str;

	// TODO this should somehow check if title and description exist and don't print the message if it doesn't exist
	args->m_user->notify(TNT_STATUS, args, "<yellow>$this.title");
	args->m_user->notify(TNT_STATUS, args, "<lightgreen>$this.description");
	/// Print the views of all objects contained in the target
	//for (cur = args->m_this->contents(); cur; cur = cur->next()) {
	//	if (cur == args->m_user)
	//		continue;
		//cur->do_action(args->m_user, args->m_channel, "print_view");
	//}

	// TODO you should print a different set of messages if the look failed
	// TODO you can transfer a lot of this "message response" code to a special function which takes a list of
	//	responses and prints them under the various conditions

	/// Print a message to all objects in the room that someone is looking at something
	//for (cur = args->m_user->location()->contents(); cur; cur = cur->next()) {
	//	if (cur == args->m_user)	/// Don't print a message to the user
	//		continue;
	//	if (args->m_user == args->m_this)
	//		//print "checks out self" message to cur
	//		str = (MooString *) args->m_this->get_property("self_looked_at", &moo_string_obj_type);
	//	else if (cur == args->m_this)
	//		//print "victimized message"
	//		//(if someone looks at you, the message should come from whatever is looking at you
	//		str = (MooString *) args->m_user->get_property("look_at", &moo_string_obj_type);
	//	else
	//		//print "3rd party" message
	//		//("You see $user.name looking around the room confused")
	//		//("$user.name examines a $this.name")
	//		//This comes from the object being looked at
	//		str = (MooString *) args->m_this->get_property("looked_at", &moo_string_obj_type);
	//	if (str)
	//		cur->notify(TNT_STATUS, args, str->m_str);
	//}
	return(0);
}

static int basics_go(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
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

/*
static int basics_respond(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{


	/// Print a message to all objects in the room that someone is looking at something
	for (cur = args->m_user->location()->contents(); cur; cur = cur->next()) {
		if (!victim) {
			if (cur == args->m_user)
				str = responses->get_string(MOO_STR_NULL_SELF);
			else
				str = responses->get_string(MOO_STR_NULL_OTHERS);
		}
		else if (victim == args->m_user) {
			if (cur == args->m_user)
				str = responses->get_string(MOO_STR_AUTO_SELF);
			else
				str = responses->get_string(MOO_STR_AUTO_OTHERS);
		}
		else {
			if (cur == args->m_user)
				str = responses->get_string(MOO_STR_VICT_SELF);
			else if (cur == victim)
				str = responses->get_string(MOO_STR_VICT_VICT);
			else
				str = responses->get_string(MOO_STR_VICT_OTHERS);
		}
		if (str && *str != '\0')
			cur->notify(TNT_STATUS, args, str);
	}
}
*/

int moo_load_basic_actions(MooObjectHash *env)
{
	env->set("basics_register", new MooCodeFunc(basics_register));
	env->set("basics_print", new MooCodeFunc(basics_print));
	env->set("basics_print_view", new MooCodeFunc(basics_print_view));
	env->set("basics_look_self", new MooCodeFunc(basics_look_self));
	env->set("basics_go", new MooCodeFunc(basics_go));
	return(0);
}


