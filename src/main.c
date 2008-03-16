/*
 * Module Name:	main.c
 * Description:	Mud
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sdm/data.h>
#include <sdm/timer.h>
#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>
#include <sdm/interface/telnet.h>

#include <sdm/modules/module.h>
#include <sdm/modules/basic/basic.h>
#include <sdm/modules/sdrl/sdrl.h>

#include <sdm/objs/user.h>
#include <sdm/objs/world.h>
#include <sdm/objs/interpreter.h>

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
	if (init_module() < 0)
		return(-1);
	if (init_basic() < 0)
		return(-1);
	if (init_sdrl() < 0)
		return(-1);

	if (init_interpreter() < 0)
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

int release_mud(void)
{
	release_user();
	release_world();
	release_thing();
	release_interpreter();

	release_sdrl();
	release_basic();
	release_module();
	release_object();

	release_telnet();
	release_interface();
	release_timer();

	release_data();
	return(0);
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


