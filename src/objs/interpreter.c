/*
 * Object Name:	interpreter.c
 * Description:	Command Interpreter Object
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sdm/text.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>
#include <sdm/objs/interpreter.h>

struct sdm_command {
	sdm_command_t func;
	void *ptr;
	destroy_t destroy;
};

struct sdm_processor_type sdm_interpreter_obj_type = { {
	(struct sdm_object_type *) &sdm_processor_obj_type,
	sizeof(struct sdm_interpreter),
	NULL,
	(sdm_object_init_t) sdm_interpreter_init,
	(sdm_object_release_t) sdm_interpreter_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL	},
	(sdm_processor_startup_t) sdm_interpreter_startup,
	(sdm_processor_process_t) sdm_interpreter_process,
	(sdm_processor_shutdown_t) sdm_interpreter_shutdown
};

static struct sdm_hash *global_commands = NULL;

int sdm_cmd_quit(void *, struct sdm_user *, char *);
static void destroy_sdm_command(struct sdm_command *);

int init_interpreter(void)
{
	// TODO how will you store commands?  will you need a destroy function here?
	if (!(global_commands = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, -1, (destroy_t) destroy_sdm_command)))
		return(-1);

	// TODO should the quit command go elsewhere too?
	sdm_interpreter_add(NULL, "quit", sdm_cmd_quit, NULL, NULL);
	return(0);
}

void release_interpreter(void)
{
	if (!global_commands)
		return;
	destroy_sdm_hash(global_commands);
	global_commands = NULL;
}

int sdm_interpreter_init(struct sdm_interpreter *interpreter, va_list va)
{
	if (!(interpreter->commands = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, -1, NULL)))
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
	// TODO print motd

	// TODO this will be handled in the "on_enter" action
	//sdm_thing_do_action(SDM_THING(SDM_THING(user)->location), user, "look", "");
	SDM_INTERFACE_WRITE(user->inter, SDM_TXT_COMMAND_PROMPT);
	return(0);
}

int sdm_interpreter_process(struct sdm_interpreter *proc, struct sdm_user *user, char *input)
{
	int i, res;
	struct sdm_thing *obj;
	struct sdm_command *cmd;

	/** Isolate the command */
	for (i = 0; (input[i] != ' ') && (input[i] != '\0'); i++) ;
	if (input[i] != '\0') {
		input[i] = '\0';
		i++;
	}

	// TODO should you automatically grap the object from the args instead of passing null to do_action?
	if ((cmd = (struct sdm_command *) sdm_hash_find(global_commands, input)))
		res = cmd->func(cmd->ptr, user, &input[i]);
	else if (((res = sdm_thing_do_action(SDM_THING(user), SDM_THING(user), input, NULL, &input[i])) != 0)
	    && ((res = (sdm_thing_do_action(SDM_THING(SDM_THING(user)->location), SDM_THING(user), input, NULL, &input[i])) != 0)
	    && ((obj = sdm_interpreter_find_object(user, &input[i], &i))))) {
		res = sdm_thing_do_action(obj, SDM_THING(user), input, NULL, &input[i]);
	}
 	if (res == SDM_CMD_CLOSE)
		return(1);
	if (res > 0)
		SDM_INTERFACE_WRITE(user->inter, SDM_TXT_COMMAND_NOT_FOUND);
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


struct sdm_thing *sdm_interpreter_find_object(struct sdm_user *user, const char *str, int *used)
{
	sdm_id_t id;
	int i, j = 0;
	char buffer[STRING_SIZE];

	for (i = 0; (str[i] == ' ') && (str[i] != '\0'); i++) ;
	if (str[i] == '\"')
		for (; (i < STRING_SIZE) && (str[i] != '\"') && (str[i] != '\0'); i++)
			buffer[j++] = str[i];
	else
		for (; (i < STRING_SIZE) && (str[i] != ' ') && (str[i] != '\0'); i++)
			buffer[j++] = str[i];
	buffer[j] = '\0';
	if (used)
		*used += i;
	if (buffer[0] == '#') {
		id = atoi(&buffer[1]);
		return(sdm_thing_lookup_id(id));
	}
	if (!SDM_THING(user)->location)
		return(NULL);
	return(sdm_container_find(SDM_THING(user)->location, buffer));
}


/*** Global Commands ***/

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


