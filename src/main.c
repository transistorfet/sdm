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

#include <sdm/modules/basic/basic.h>
#include <sdm/modules/lua/lua.h>

#include <sdm/objs/user.h>
#include <sdm/objs/world.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

static int exit_flag = 1;

int serverloop(void);

static void handle_sigint(int);

int init_mud(void)
{
	if (init_data() < 0)
		return(-1);
	sdm_set_data_path("data");

	if (init_timer() < 0)
		return(-1);
	if (init_interface() < 0)
		return(-1);
	if (init_telnet() < 0)
		return(-1);

	if (init_object() < 0)
		return(-1);
	if (init_sdm_number_type() < 0)
		return(-1);
	if (init_sdm_string_type() < 0)
		return(-1);

	if (init_basic() < 0)
		return(-1);
	if (init_lua() < 0)
		return(-1);

	if (init_thing() < 0)
		return(-1);
	if (init_world() < 0)
		return(-1);
	if (init_user() < 0)
		return(-1);

	signal(SIGINT, handle_sigint);
	return(0);
}

void release_mud(void)
{
	release_user();
	release_world();
	release_thing();

	release_lua();
	release_basic();

	release_sdm_string_type();
	release_sdm_number_type();
	release_object();

	release_telnet();
	release_interface();
	release_timer();

	release_data();
}

/**
 * Main Entry Point
 */
int main(int argc, char **argv)
{
	if (init_mud()) {
		printf("Failed to initialize mud\n");
		release_mud();
		return(0);
	}

	serverloop();

	release_mud();
	return(0);
}

int serverloop(void)
{
	while (exit_flag) {
		sdm_interface_select(1);
		sdm_timer_check();
	}
	return(0);
}

static void handle_sigint(int signum)
{
	printf("\nShutting down...\n");
	exit_flag = 0;
}


