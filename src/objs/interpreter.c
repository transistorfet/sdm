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
#include <sdm/objs/string.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>
#include <sdm/objs/interpreter.h>

struct sdm_processor_type sdm_interpreter_obj_type = { {
	(struct sdm_object_type *) &sdm_processor_obj_type,
	"interpreter",
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

static struct sdm_thing *sdm_interpreter_find_object(struct sdm_thing *, const char *);

int sdm_interpreter_init(struct sdm_interpreter *interpreter, va_list va)
{
	return(0);
}

void sdm_interpreter_release(struct sdm_interpreter *interpreter)
{

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

	if (input[0] == '\0')
		return(0);
	// TODO if you change the actions lookup thingamabob to a binary tree, you can have it do a best
	//	match lookup which would allow abrev'd commands and commands with spaces
	/** Isolate the command */
	for (i = 0; (input[i] != ' ') && (input[i] != '\0'); i++) ;
	if (input[i] != '\0') {
		input[i] = '\0';
		i++;
	}

	// TODO should you automatically grab the object from the args instead of passing null to do_action?
	if (!strcmp(input, "quit"))
		return(1);
	else if (((res = sdm_thing_do_action(SDM_THING(user), SDM_THING(user), input, NULL, &input[i], NULL)) > 0)
	    && ((res = (sdm_thing_do_action(SDM_THING(SDM_THING(user)->location), SDM_THING(user), input, NULL, &input[i], NULL)) > 0)
	    && ((obj = sdm_interpreter_get_thing(SDM_THING(user), &input[i], &i))))) {
		res = sdm_thing_do_action(obj, SDM_THING(user), input, NULL, &input[i], NULL);
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


int sdm_interpreter_get_string(const char *str, char *buffer, int max, int *used)
{
	int i, j = 0;

	max--;
	if (str[0] == '\"')
		for (i = 0; (i < max) && (str[i] != '\"') && (str[i] != '\0'); i++)
			buffer[j++] = str[i];
	else
		for (i = 0; (i < max) && (str[i] != ' ') && (str[i] != '\0'); i++)
			buffer[j++] = str[i];
	buffer[j] = '\0';
	if (used)
		*used += i;
	return(j);
}

struct sdm_thing *sdm_interpreter_get_thing(struct sdm_thing *thing, const char *str, int *used)
{
	sdm_id_t id;
	struct sdm_thing *obj;
	char buffer[STRING_SIZE];

	str = TRIM_WHITESPACE(str);
	sdm_interpreter_get_string(str, buffer, STRING_SIZE, used);
	if (buffer[0] == '#') {
		id = atoi(&buffer[1]);
		return(sdm_thing_lookup_id(id));
	}
	if (!strcasecmp(buffer, "me"))
		return(thing);
	if (!strcasecmp(buffer, "here"))
		return(thing->location);
	if ((obj = sdm_interpreter_find_object(thing, buffer)))
		return(obj);
	if (!thing->location)
		return(NULL);
	return(sdm_interpreter_find_object(thing->location, buffer));
}

/*** Local Functions ***/

static struct sdm_thing *sdm_interpreter_find_object(struct sdm_thing *thing, const char *name)
{
	int len;
	const char *str;
	struct sdm_thing *cur;

	// TODO modify to do a partial string match given the whole command line so that object names with
	//	spaces will match without the need for quoting them
	len = strlen(name);
	for (cur = thing->objects; cur; cur = cur->next) {
		if (!(str = sdm_thing_get_string_property(cur, "name")))
			continue;
		if (!strncasecmp(str, name, len))
			return(cur);
	}
	return(NULL);
}


