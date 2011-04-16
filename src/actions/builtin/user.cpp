/*
 * Name:	user.cpp
 * Description:	User Actions
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


static int user_init(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO set any properties or whatever needs to be found on any user thing
	return(0);
}

static int user_passwd(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO change the password for the user
	return(0);
}

static int user_notify(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int type;
	MooObject *obj;
	MooUser *thing;
	const char *name;
	MooThing *user, *channel;
	char buffer[LARGE_STRING_SIZE];

	if (!(thing = dynamic_cast<MooUser *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 3)
		throw moo_args_mismatched;
	// TODO this is temporary until you get notify refactored
	name = args->m_args->get_string(0);
	user = args->m_args->get_thing(1);
	channel = args->m_args->get_thing(2);
	if (!(obj = args->m_args->get(3)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (!strcmp(name, "status"))
		type = TNT_STATUS;
	else if (!strcmp(name, "join"))
		type = TNT_JOIN;
	else if (!strcmp(name, "leave"))
		type = TNT_LEAVE;
	else if (!strcmp(name, "say"))
		type = TNT_SAY;
	else if (!strcmp(name, "emote"))
		type = TNT_EMOTE;
	else if (!strcmp(name, "quit"))
		type = TNT_QUIT;
	else
		throw MooException("arg 0: Invalid notify() type");
	thing->notify(type, user, channel, buffer);
	return(0);
}

static int user_inventory(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	const char *str;
	struct sdm_thing *cur;

	if (!thing->objects) {
		sdm_notify(args->caller, args, "<brightgreen>You aren't carrying anything.\n");
		return(0);
	}

	sdm_notify(args->caller, args, "<brightgreen>You are carrying:\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (!(str = sdm_get_string_property(cur, "title")))
			continue;
		sdm_notify(args->caller, args, "<brightblue>    %s.\n", str);
	}
*/
	return(0);
}

int moo_load_user_actions(MooObjectHash *env)
{
	env->set("user_init", new MooCodeFunc(user_init));
	env->set("user_passwd", new MooCodeFunc(user_passwd));
	env->set("user_notify", new MooCodeFunc(user_notify));
	env->set("user_inventory", new MooCodeFunc(user_inventory));
	return(0);
}


