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


static int user_notify(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int type;
	MooObject *obj;
	MooUser *thing;
	MooThing *user, *channel;
	char buffer[LARGE_STRING_SIZE];

	if (!(thing = dynamic_cast<MooUser *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 3)
		throw moo_args_mismatched;
	type = args->m_args->get_integer(0);
	user = args->m_args->get_thing(1);
	channel = args->m_args->get_thing(2);
	if (!(obj = args->m_args->get(3)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (type < TNT_FIRST || type > TNT_LAST)
		throw MooException("arg 0: Invalid notify() type");
	thing->notify(type, user, channel, buffer);
	return(0);
}

static int thing_load(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != -1)
		throw moo_args_mismatched;
	thing->load();
	return(0);
}

static int thing_save(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != -1)
		throw moo_args_mismatched;
	thing->save();
	return(0);
}

static int thing_save_all(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO permissions check
	MooThing::save_all();
	return(0);
}

static int thing_clone(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *func;
	MooObject *id = NULL;
	MooThing *thing, *parent;
	moo_id_t idnum = MOO_NEW_ID;

	// TODO is this good being a method? will we run into problems with initializing objects?
	if (!(parent = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() == 0)
		func = args->m_args->get(0);
	else if (args->m_args->last() == 1) {
		id = args->m_args->get(0);
		func = args->m_args->get(1);
	}
	else
		throw moo_args_mismatched;
	if (id)
		idnum = id->get_integer();

	// TODO permissions check!!!
	thing = parent->clone(idnum);
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), new MooCodeExpr(0, 0, MCT_OBJECT, thing, NULL)));
	if (func)
		frame->push_call(env, new MooMethod(thing, func), new MooArgs());
	return(0);
}

static int thing_move(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	MooThing *thing, *where;

	// TODO anything special for permissions?
	if (!(thing = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	if (!(obj = args->m_args->get(0)) || !(where = obj->get_thing()))
		throw moo_type_error;
	return(thing->move(where));
}

#define MAX_WORDS	256

static int parse_command(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int i = 0, j = 0;
	MooObject *method, *user;
	const char *text, *argstr;
	char *words[MAX_WORDS];
	MooArgs *newargs;
	MooObjectHash *newenv;

	if (!(user = env->get("user")))
		throw MooException("No user object set.");
	text = args->m_args->get_string(0);
	argstr = parser_next_word(text);

/*
	strncpy(buffer, text, LARGE_STRING_SIZE);
	buffer[LARGE_STRING_SIZE] = '\0';
	while (parser_is_whitespace(buffer[i]))
		i++;
	// TODO alternatively, you can send this all to the lisp parser and traverse the expr you get back.
	//	If you come across a CALL, convert it to a string or do something else with it so that you end up
	//	with a list of numbers, strings (including code in string form), and identifiers.
	//	Identifiers would also actually be strings, unless perhaps they were of a certain form, such as #<num>
	//	You couldn't directly try to resolve all identifiers into values because some barewords for commands are
	//	intended to be strings
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
*/

	newargs = new MooArgs();
	newenv = new MooObjectHash(env);
	newenv->set("argstr", new MooString("%s", argstr));

	MooCodeExpr *expr;

	if (!(expr = MooCodeParser::parse_code(text)))
		MooException("Unable to parse input.");
	for (MooCodeExpr *cur = expr; cur; cur = cur->next()) {
		switch (cur->expr_type()) {
		    case MCT_OBJECT:
			newargs->m_args->push(cur->value());
			break;
		    case MCT_IDENTIFIER:
			newargs->m_args->push(new MooString("%s", cur->get_identifier()));
			break;
		    case MCT_CALL: {
			char buffer[LARGE_STRING_SIZE];
			MooCodeParser::generate(dynamic_cast<MooCodeExpr *>(cur->value()), buffer, LARGE_STRING_SIZE, &moo_style_one_line);
			newargs->m_args->push(new MooString("%s", buffer));
			break;
		    }
		    default:
			break;
		}
	}

	MooObject *obj;
	const char *cmd;

	if (!(obj = newargs->m_args->shift()) || !(cmd = obj->get_string()))
		throw MooException("No command given");
	frame->push_debug("> in realm_command: %s", cmd);

	if (!(method = user->resolve_method(cmd))) {
		MooObject *location = user->resolve_property("location");
		if (!(method = location->resolve_method(cmd))) {
			// TODO try to parse more and search the objects
		}
	}

	// TODO you could push markers onto the stack like here so that if an exception occurs inside of the call we are pushing, then
	//	it will print that it occurred inside of this evaluate function.

	frame->push_call(newenv, method, newargs);

	// TODO you could have a call here to an optional method on the user after a command has been executed (like a prompt)

	return(0);
}

int moo_load_thing_methods(MooObjectHash *env)
{
	env->set("N/STATUS", new MooNumber((long int) TNT_STATUS));
	env->set("N/JOIN", new MooNumber((long int) TNT_JOIN));
	env->set("N/LEAVE", new MooNumber((long int) TNT_LEAVE));
	env->set("N/SAY", new MooNumber((long int) TNT_SAY));
	env->set("N/EMOTE", new MooNumber((long int) TNT_EMOTE));
	env->set("N/QUIT", new MooNumber((long int) TNT_QUIT));

	env->set("%user_notify", new MooFunc(user_notify));
	env->set("%thing_load", new MooFunc(thing_load));
	env->set("%thing_save", new MooFunc(thing_save));
	env->set("%thing_save_all", new MooFunc(thing_save_all));
	env->set("%thing_clone", new MooFunc(thing_clone));
	env->set("%thing_move", new MooFunc(thing_move));
	env->set("%parse_command", new MooFunc(parse_command));
	return(0);
}


