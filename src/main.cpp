/*
 * Module Name:	main.c
 * Description:	Mud
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sdm/data.h>
#include <sdm/timer.h>
#include <sdm/exception.h>
#include <sdm/code/code.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/irc/pseudoserv.h>

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/objs/config.h>
#include <sdm/objs/array.h>
#include <sdm/objs/hash.h>
#include <sdm/funcs/func.h>
#include <sdm/funcs/method.h>
#include <sdm/things/channel.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

static int exit_flag = 1;

int serverloop(void);

static void handle_sigint(int);

extern int moo_load_basic_funcs(MooObjectHash *env);
extern int moo_load_thing_methods(MooObjectHash *env);

int init_moo(void)
{
	try {
		init_data();
		moo_set_data_path("data");

		init_object();

		moo_object_register_type(&moo_number_obj_type);
		moo_object_register_type(&moo_string_obj_type);
		moo_object_register_type(&moo_thingref_obj_type);
		init_array();
		init_hash();

		moo_object_register_type(&moo_func_obj_type);
		moo_object_register_type(&moo_method_obj_type);

		init_timer();
		init_task();
		init_interface();
		init_irc_pseudoserv();

		init_moo_code();

		moo_load_basic_funcs(global_env);
		moo_load_thing_methods(global_env);

		moo_object_register_type(&moo_tcp_obj_type);
		moo_object_register_type(&moo_irc_pseudoserv_obj_type);

		init_thing();
		init_user();
		//init_channel();
		//init_world();

		load_global_config();

		// TODO what should we use as the name mangle to signify global values (@ isn't very good becuase it's used by funcs)
		global_env->set("@start", new MooThingRef(MooThing::reference(MOO_START_ROOM)));
		global_env->set("@user", new MooThingRef(MooThing::reference(MOO_GENERIC_USER)));
		global_env->set("@room", new MooThingRef(MooThing::reference(MOO_GENERIC_ROOM)));
		global_env->set("@exit", new MooThingRef(MooThing::reference(MOO_GENERIC_EXIT)));
		global_env->set("@mobile", new MooThingRef(MooThing::reference(MOO_GENERIC_MOBILE)));
		global_env->set("@cryolocker", new MooThingRef(MooThing::reference(MOO_CRYOLOCKER)));
		global_env->set("@channels", new MooThingRef(MooThing::reference(MOO_CHANNELS)));
	}
	catch (MooException e) {
		moo_status("%s", e.get());
		return(-1);
	}

	signal(SIGINT, handle_sigint);
	return(0);
}

void release_moo(void)
{
	release_user();
	//release_world();
	//release_channel();
	release_thing();

	release_moo_code();

	release_array();
	release_hash();

	release_irc_pseudoserv();
	release_interface();
	release_task();
	release_timer();
	release_object();

	release_data();
}

/**
 * Main Entry Point
 */
int main(int argc, char **argv)
{
	if (init_moo()) {
		printf("Failed to initialize mud\n");
		release_moo();
		return(0);
	}

	serverloop();

	release_moo();
	return(0);
}

int serverloop(void)
{
	while (exit_flag) {
		MooInterface::wait(1);
		MooTimer::check();
	}
	return(0);
}

static void handle_sigint(int signum)
{
	printf("\nShutting down...\n");
	exit_flag = 0;
}


