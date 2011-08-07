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
#include <sdm/objs/thing.h>

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
	MooMethod *init;
	MooObject *func;
	MooObject *id = NULL;
	MooThing *thing, *parent;
	moo_id_t idnum = MOO_NEW_ID;

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

	/// Call the 'initialize' method of each parent object (Most distant parent will be called first)
	while (parent) {
		if ((init = dynamic_cast<MooMethod *>(parent->resolve_method("initialize")))) {
			init->m_obj = thing;
			frame->push_call(env, init, new MooArgs());
		}
		parent = parent->parent();
	}
	return(0);
}

#define MAX_WORDS	256
#define PREPOSITIONS	5
const char *prepositions[] = { "to", "in", "from", "is", "as" };

static int parse_command(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int k, m;
	int i = 0, j = 0;
	MooObject *method, *user;
	const char *text, *cmd, *argstr;
	char *words[MAX_WORDS];
	MooArgs *newargs;
	MooObjectHash *newenv;
	char buffer[LARGE_STRING_SIZE];

	if (!(user = env->get("user")))
		throw MooException("No user object set.");
	text = args->m_args->get_string(0);
	argstr = parser_next_word(text);

	// Parse the text into words
	strncpy(buffer, text, LARGE_STRING_SIZE);
	buffer[LARGE_STRING_SIZE] = '\0';
	while (parser_is_whitespace(buffer[i]))
		i++;
	words[0] = &buffer[i];
	for (; buffer[i] != '\0'; i++) {
		if (buffer[i] == '\"') {
			words[j] = &buffer[++i];
			for (; buffer[i] != '\0' && buffer[i] != '\"'; i++)
				;
		}

		if (buffer[i] == '\"' || parser_is_whitespace(buffer[i])) {
			buffer[i++] = '\0';
			while (parser_is_whitespace(buffer[i]))
				i++;
			words[++j] = &buffer[i];
		}
	}

	/// Build the arguments
	newargs = new MooArgs();
	newenv = new MooObjectHash(env);
	cmd = words[0];
	newenv->set("argstr", new MooString("%s", argstr));
	for (k = 1; k < j; k++)
		newargs->m_args->push(new MooString("%s", words[k]));

	for (k = 1; k < j; k++) {
		for (m = 0; m < PREPOSITIONS; m++)
			if (!strcasecmp(words[k], prepositions[m]))
				break;
	}
	newenv->set("prep", new MooString("%s", (k < j) ? words[k++] : ""));

	if (!(method = user->resolve_method(cmd))) {
		MooObject *location = user->resolve_property("location");
		if (location && !(method = location->resolve_method(cmd))) {
			// TODO try to parse more and search the objects
		}
	}

	frame->push_debug("> in realm_command: %s", cmd);
	frame->push_call(newenv, method, newargs);

	// TODO you could have a call here to an optional method on the user after a command has been executed (like a prompt)

	return(0);

/*
	(define this:command (lambda (text)
		(define words (***parse-into-words***)
		(define dobj "")
		(define prep "")
		(define iobj "")
		(words:foreach (lambda (cur)
			(cond
				((!= (prepositions:search cur) -1)
					(set! prep cur))
				((equal? prep "")
					(set! dobj (concat dobj " " cur)))
				(else
					(set! iobj (concat iobj " " cur)))
		))
	))
*/


}

int moo_load_thing_methods(MooObjectHash *env)
{
	env->set("%thing_load", new MooFunc(thing_load));
	env->set("%thing_save", new MooFunc(thing_save));
	env->set("%thing_save_all", new MooFunc(thing_save_all));
	env->set("%thing_clone", new MooFunc(thing_clone));
	env->set("%parse_command", new MooFunc(parse_command));
	return(0);
}


