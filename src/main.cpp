/*
 * Module Name:	main.c
 * Description:	Mud
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sdm/data.h>
#include <sdm/timer.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/telnet.h>

//#include <sdm/actions/lua/lua.h>
#include <sdm/actions/builtin/builtin.h>

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/config.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

static int exit_flag = 1;

int serverloop(void);

static void handle_sigint(int);

int init_moo(void)
{
	if (init_data() < 0)
		return(-1);
	moo_set_data_path("data");

	if (init_object() < 0)
		return(-1);
	if (init_timer() < 0)
		return(-1);
	if (init_task() < 0)
		return(-1);
	if (init_interface() < 0)
		return(-1);
//	if (init_telnet() < 0)
//		return(-1);

	if (init_moo_number_type() < 0)
		return(-1);
	if (init_moo_string_type() < 0)
		return(-1);

//	if (init_builtin() < 0)
//		return(-1);
//	if (init_lua() < 0)
//		return(-1);

	if (init_thing() < 0)
		return(-1);
	if (init_world() < 0)
		return(-1);
	if (init_user() < 0)
		return(-1);
	if (load_global_config())
		return(-1);

	signal(SIGINT, handle_sigint);
	return(0);
}

void release_moo(void)
{
	release_user();
	release_world();
	release_thing();

//	release_lua();
//	release_builtin();

	release_moo_string_type();
	release_moo_number_type();

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


