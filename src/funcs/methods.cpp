/*
 * Name:	methods.cpp
 * Description:	Basic Methods
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>


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

static int builder_save(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooWorld *root;

	if (!(root = MooWorld::root()))
		return(-1);
	root->write();
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



int moo_load_basic_methods(MooObjectHash *env)
{
	env->set("user_notify", new MooFunc(user_notify));
	env->set("builder_save", new MooFunc(builder_save));
	// TODO what's a better name than evaluate
	env->set("channel_evaluate", new MooFunc(channel_evaluate));
	env->set("realm_evaluate", new MooFunc(realm_evaluate));
	return(0);
}


