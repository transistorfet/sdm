/*
 * Module Name:	main.c
 * Description:	Mud
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sdm/data.h>
#include <sdm/commands.h>
#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>
#include <sdm/interface/telnet.h>

static int exit_flag = 1;

int serverloop(void);

static void handle_sigint(int);

int init_mud(void)
{
	if (init_data() < 0)
		return(-1);
	sdm_set_data_path("../data");

	if (init_interface() < 0)
		return(-1);
	if (init_telnet() < 0)
		return(-1);

	if (init_user() < 0)
		return(-1);
	if (init_commands() < 0)
		return(-1);

	signal(SIGINT, handle_sigint);
	return(0);
}

int release_mud(void)
{
	release_user();
	release_telnet();
	release_interface();
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
		//fe_timer_check();
	}
	return(0);
}

static void handle_sigint(int signum)
{
	printf("\nShutting down...\n");
	exit_flag = 0;
}


