/*
 * Object Name:	interpreter.c
 * Description:	Command Interpreter Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/text.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/interface/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>
#include <sdm/objs/interpreter.h>

struct sdm_command {
	sdm_command_t func;
	void *ptr;
	destroy_t destroy;
};

struct sdm_processor_type sdm_interpreter_obj_type = {
    {
	(struct sdm_object_type *) &sdm_processor_obj_type,
	sizeof(struct sdm_interpreter),
	NULL,
	(sdm_object_init_t) sdm_interpreter_init,
	(sdm_object_release_t) sdm_interpreter_release
    },
	(sdm_processor_startup_t) sdm_interpreter_startup,
	(sdm_processor_process_t) sdm_interpreter_process,
	(sdm_processor_shutdown_t) sdm_interpreter_shutdown
};

static struct sdm_hash *global_commands = NULL;

int sdm_cmd_say(void *, struct sdm_user *, char *);
int sdm_cmd_quit(void *, struct sdm_user *, char *);

static void destroy_sdm_command(struct sdm_command *);

int init_interpreter(void)
{
	// TODO how will you store commands?  will you need a destroy function here?
	if (!(global_commands = create_sdm_hash(0, (destroy_t) destroy_sdm_command)))
		return(-1);

	// TODO this is only here temporarily
	sdm_interpreter_add(NULL, "say", sdm_cmd_say, NULL, NULL);
	sdm_interpreter_add(NULL, "quit", sdm_cmd_quit, NULL, NULL);

	return(0);}

int release_interpreter(void)
{
	if (global_commands)
		destroy_sdm_hash(global_commands);
	return(0);
}

int sdm_interpreter_init(struct sdm_interpreter *interpreter, va_list va)
{
	if (!(interpreter->commands = create_sdm_hash(SDM_BBF_CASE_INSENSITIVE, NULL)))
		return(-1);
	return(0);
}

void sdm_interpreter_release(struct sdm_interpreter *interpreter)
{
	if (interpreter->commands)
		destroy_sdm_hash(interpreter->commands);
}


int sdm_interpreter_startup(struct sdm_interpreter *proc, struct sdm_user *user)
{
	SDM_INTERFACE_WRITE(user->inter, SDM_TXT_COMMAND_PROMPT);
	return(0);
}

int sdm_interpreter_process(struct sdm_interpreter *proc, struct sdm_user *user, char *input)
{
	int i;
	char *name;
	struct sdm_command *cmd;
	struct sdm_actionable *obj;

	/*
		get command portion (up to first space)
		if command is in global commands
			evaluate command
		else
			get next portion of command line
			(note: an object name need only match what was typed to allow for shorthands)
			if name is contained by the user
				evaluate command on that object
			else if name is contained by the owner of user (room user is in)
				evaluate command on that object
			else
				command not found
	*/

	/** Isolate the command */
	for (i = 0; (input[i] != ' ') && (input[i] != '\0'); i++) ;
	if (input[i] != '\0') {
		input[i] = '\0';
		i++;
	}

	/** Evaluate the command if it's in global_commands */
	if ((cmd = (struct sdm_command *) sdm_hash_find(global_commands, input))) {
		if (cmd->func(cmd->ptr, user, &input[i]) == SDM_CMD_CLOSE)
			return(-1);
	}
	else {
		/** Isolate the object name */
		for (; (input[i] == ' ') && (input[i] != '\0'); i++) ;
		name = &input[i];
		if (input[i] == '\"')
			for (; (input[i] != '\"') && (input[i] != '\0'); i++) ;
		else
			for (; (input[i] != ' ') && (input[i] != '\0'); i++) ;
		input[i] = '\0';
		if (input[i] != '\0') {
			input[i] = '\0';
			i++;
		}
		/** Evaluate the command as an action on the object */
		if (*name == '\0') {
			SDM_INTERFACE_WRITE(user->inter, SDM_TXT_COMMAND_NOT_FOUND);
		}
		else if ((obj = sdm_container_find(SDM_ACTIONABLE(user)->owner, name))) {
			if (sdm_actionable_do_action(obj, user, input, &input[i]) < 0)
				sdm_user_tell(user, SDM_TXT_ACTION_NOT_FOUND);
		}
		else {
			sdm_user_tell(user, SDM_TXT_OBJECT_NOT_FOUND, name);
		}
	}
	SDM_INTERFACE_WRITE(user->inter, SDM_TXT_COMMAND_PROMPT);
	return(0);
}

int sdm_interpreter_shutdown(struct sdm_interpreter *proc, struct sdm_user *user)
{
	return(0);
}


int sdm_interpreter_add(struct sdm_interpreter *proc, const char *name, sdm_command_t func, void *ptr, destroy_t destroy)
{
	int res;
	struct sdm_command *cmd;

	if (!func)
		return(-1);
	if (!(cmd = (struct sdm_command *) memory_alloc(sizeof(struct sdm_command))))
		return(-1);
	cmd->func = func;
	cmd->ptr = ptr;
	cmd->destroy = destroy;

	if (proc)
		res = sdm_hash_add(proc->commands, name, cmd);
	else
		res = sdm_hash_add(global_commands, name, cmd);

	if (res >= 0)
		return(0);
	memory_free(cmd);
	return(-1);
}

/*** Global Commands ***/

// TODO where should this really go?
int sdm_cmd_say(void *ptr, struct sdm_user *user, char *args)
{
	if (!args || *args == '\0')
		return(-1);
	sdm_user_tell(user, "You say \"%s\"\n", args);
	sdm_user_announce(user, "%s says \"%s\"\n", user->name, args);
	return(0);
}

int sdm_cmd_quit(void *ptr, struct sdm_user *user, char *args)
{
	return(SDM_CMD_CLOSE);
}

/*** Local Functions ***/

static void destroy_sdm_command(struct sdm_command *cmd)
{
	if (cmd->destroy)
		cmd->destroy(cmd->ptr);
	memory_free(cmd);
}


