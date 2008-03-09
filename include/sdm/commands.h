/*
 * Header Name:	commands.h
 * Description:	Command Interpreter Header
 */

#ifndef _SDM_COMMANDS_H
#define _SDM_COMMANDS_H

#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/user.h>

typedef int (*sdm_command_t)(void *, struct sdm_user *, char *);

struct sdm_command {
	sdm_command_t func;
	void *ptr;
	destroy_t destroy;
};

int init_commands(void);
int release_commands(void);

int sdm_add_command(const char *, sdm_command_t, void *, destroy_t);

// TODO should the command be const char?
int sdm_evaluate_command(struct sdm_user *, char *);

#endif

