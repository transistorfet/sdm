/*
 * Name:	moocode.c
 * Description:	MooCode Actions
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


static int moocode_eval(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	return(frame->push_code(args->m_args->get_string(0), args));
}

int moo_load_moocode_actions(MooObjectHash *env)
{
	env->set("moocode_eval", new MooCodeFunc(moocode_eval));
	return(0);
}

