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


static int moocode_eval(MooAction *action, MooThing *thing, MooArgs *args)
{
	int ret;
	const char *text;
	MooCodeExpr *code;
	MooObjectHash *env;
	MooCodeFrame frame;
	MooCodeParser parser;

	text = args->m_args->get_string(0);
	if (*text == '\0')
		return(-1);
	try {
		code = parser.parse(text);
		// TODO temporary
		{ char buffer[1024];
			MooCodeParser::generate(code, buffer, 1024);
			moo_status("CODE: %s", buffer);
		}
	}
	catch (MooException e) {
		moo_status("%s", e.get());
		return(-1);
	}

	env = frame.env();
	// TODO add args to the environment (as MooArgs, or as something else?)
	//env->set("args", args);
	//env->set("parent", new MooThingRef(m_thing));
	frame.add_block(args, code);
	ret = frame.run();
	args->m_result = frame.get_return();
	return(ret);
}

int moo_load_moocode_actions(MooBuiltinHash *actions)
{
	actions->set("moocode_eval", new MooBuiltin(moocode_eval));
	return(0);
}

