/*
 * Name:	code.cpp
 * Description:	MooCode
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/code/code.h>

MooObjectHash *global_env = NULL;

extern int moo_load_code_basic(MooObjectHash *env);

extern int moo_load_moocode_actions(MooObjectHash *env);
extern int moo_load_basic_actions(MooObjectHash *env);
extern int moo_load_builder_actions(MooObjectHash *env);
extern int moo_load_channel_actions(MooObjectHash *env);
extern int moo_load_item_actions(MooObjectHash *env);
extern int moo_load_mobile_actions(MooObjectHash *env);
extern int moo_load_room_actions(MooObjectHash *env);
extern int moo_load_user_actions(MooObjectHash *env);

int init_moo_code(void)
{
	moo_object_register_type(&moo_code_func_obj_type);
	moo_object_register_type(&moo_code_expr_obj_type);
	moo_object_register_type(&moo_code_frame_obj_type);
	moo_object_register_type(&moo_code_lambda_obj_type);
	if (global_env)
		return(1);
	global_env = new MooObjectHash();
	init_code_event();

	moo_load_code_basic(global_env);

	moo_load_moocode_actions(global_env);
	moo_load_basic_actions(global_env);
	moo_load_builder_actions(global_env);
	moo_load_channel_actions(global_env);
	moo_load_item_actions(global_env);
	moo_load_mobile_actions(global_env);
	moo_load_room_actions(global_env);
	moo_load_user_actions(global_env);
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


