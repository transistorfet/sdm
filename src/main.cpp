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
#include <sdm/array.h>
#include <sdm/hash.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/telnet.h>
#include <sdm/interfaces/rpc.h>
#include <sdm/tasks/rpc-server.h>
#include <sdm/tasks/irc/pseudoserv.h>

#include <sdm/actions/alias.h>
#include <sdm/actions/method.h>
#include <sdm/actions/builtin/builtin.h>

#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/objs/config.h>
#include <sdm/things/channel.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

static int exit_flag = 1;

int serverloop(void);

static void handle_sigint(int);

int init_moo(void)
{
	try {
		init_data();
		moo_set_data_path("data");

		init_object();
		init_timer();
		init_task();
		init_interface();
		//init_telnet();
		init_irc_pseudoserv();

		init_builtin();
		init_moo_code();

		moo_object_register_type(&moo_alias_obj_type);

		moo_object_register_type(&moo_float_obj_type);
		moo_object_register_type(&moo_integer_obj_type);
		moo_object_register_type(&moo_string_obj_type);
		moo_object_register_type(&moo_thingref_obj_type);
		init_array();
		moo_object_register_type(&moo_hash_obj_type);

		moo_object_register_type(&moo_method_obj_type);

		moo_object_register_type(&moo_tcp_obj_type);
		moo_object_register_type(&moo_rpc_obj_type);
		moo_object_register_type(&moo_rpc_server_obj_type);
		moo_object_register_type(&moo_irc_pseudoserv_obj_type);

		init_thing();
		init_channel();
		init_world();
		init_user();

		load_global_config();

		MooThing::add_global("start", new MooThingRef(MooThing::reference(MOO_START_ROOM)));
		MooThing::add_global("user", new MooThingRef(MooThing::reference(MOO_GENERIC_USER)));
		MooThing::add_global("room", new MooThingRef(MooThing::reference(MOO_GENERIC_ROOM)));
		MooThing::add_global("exit", new MooThingRef(MooThing::reference(MOO_GENERIC_EXIT)));
		MooThing::add_global("mobile", new MooThingRef(MooThing::reference(MOO_GENERIC_MOBILE)));
		MooThing::add_global("cryolocker", new MooThingRef(MooThing::reference(MOO_CRYOLOCKER)));
		MooThing::add_global("channels", new MooThingRef(MooThing::reference(MOO_CHANNELS)));
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
	release_world();
	release_channel();
	release_thing();

	release_moo_code();
	release_builtin();

	release_array();
	release_hash();

	release_irc_pseudoserv();
//	release_telnet();
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


