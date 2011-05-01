/*
 * Name:	code.cpp
 * Description:	MooCode
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/code/code.h>

MooObjectHash *global_env = NULL;

int init_moo_code(void)
{
	moo_object_register_type(&moo_code_expr_obj_type);
	moo_object_register_type(&moo_code_frame_obj_type);
	moo_object_register_type(&moo_code_lambda_obj_type);
	if (global_env)
		return(1);
	global_env = new MooObjectHash();
	global_env->set("nil", &moo_nil);
	init_code_event();
	return(0);
}

void release_moo_code(void)
{
	release_code_event();
	if (!global_env)
		return;
	delete global_env;
	global_env = NULL;
}


