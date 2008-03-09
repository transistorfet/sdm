/*
 * Module Name:	commands.c
 * Description:	Command Interpreter
 */

#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/commands.h>
#include <sdm/objs/user.h>

static struct sdm_hash *global_commands = NULL;

int sdm_cmd_quit(void *, struct sdm_user *, char *);

static void destroy_sdm_command(struct sdm_command *);

int init_commands(void)
{
	// TODO how will you store commands?  will you need a destroy function here?
	if (!(global_commands = create_sdm_hash(0, (destroy_t) destroy_sdm_command)))
		return(-1);

	sdm_add_command("quit", sdm_cmd_quit, NULL, NULL);

	return(0);
}

int release_commands(void)
{
	destroy_sdm_hash(global_commands);
	return(0);
}

int sdm_add_command(const char *name, sdm_command_t func, void *ptr, destroy_t destroy)
{
	struct sdm_command *cmd;

	if (!func)
		return(-1);
	if (!(cmd = (struct sdm_command *) memory_alloc(sizeof(struct sdm_command))))
		return(-1);
	cmd->func = func;
	cmd->ptr = ptr;
	cmd->destroy = destroy;

	return(sdm_hash_add(global_commands, name, cmd));
}


int sdm_evaluate_command(struct sdm_user *user, char *cmdline)
{
	int i;
	char *args;
	struct sdm_command *cmd;

	for (i = 0; (cmdline[i] != ' ') && (cmdline[i] != '\0'); i++)
		;
	args = (cmdline[i] != '\0') ? &cmdline[i + 1] : NULL;
	cmdline[i] = '\0';
	if (!(cmd = (struct sdm_command *) sdm_hash_find(global_commands, cmdline)))
		return(-1);
	cmd->func(cmd->ptr, user, args);
	return(0);
}


int sdm_cmd_quit(void *ptr, struct sdm_user *user, char *args)
{
	destroy_sdm_interface(user->inter);
	return(0);
}

/*** Local Functions ***/

static void destroy_sdm_command(struct sdm_command *cmd)
{
	if (cmd->destroy)
		cmd->destroy(cmd->ptr);
	memory_free(cmd);
}



