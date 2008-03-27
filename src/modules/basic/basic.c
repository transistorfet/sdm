/*
 * Name:	basic.c
 * Description:	Basic Stuff...
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/objs/action.h>
#include <sdm/objs/interpreter.h>
#include <sdm/modules/basic/basic.h>

struct sdm_object_type sdm_basic_obj_type = {
	&sdm_action_obj_type,
	"basic",
	sizeof(struct sdm_basic),
	NULL,
	(sdm_object_init_t) NULL,
	(sdm_object_release_t) NULL,
	(sdm_object_read_entry_t) sdm_basic_read_entry,
	(sdm_object_write_data_t) sdm_basic_write_data
};

static struct sdm_hash *basic_actions = NULL;

int init_basic(void)
{
	if (basic_actions)
		return(1);
	if (!(basic_actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, -1, NULL)))
		return(-1);
	if (sdm_object_register_type(&sdm_basic_obj_type) < 0)
		return(-1);

	sdm_hash_add(basic_actions, "basic_tell_user", sdm_basic_action_tell_user);
	sdm_hash_add(basic_actions, "basic_announce", sdm_basic_action_announce);
	sdm_hash_add(basic_actions, "basic_say", sdm_basic_action_say);
	sdm_hash_add(basic_actions, "basic_look", sdm_basic_action_look);
	sdm_hash_add(basic_actions, "basic_examine", sdm_basic_action_examine);
	sdm_hash_add(basic_actions, "basic_move", sdm_basic_action_move);

	sdm_hash_add(basic_actions, "basic_create_object", sdm_basic_action_create_object);
	sdm_hash_add(basic_actions, "basic_create_room", sdm_basic_action_create_room);
	sdm_hash_add(basic_actions, "basic_create_exit", sdm_basic_action_create_exit);
	return(0);
}

void release_basic(void)
{
	if (!basic_actions)
		return;
	destroy_sdm_hash(basic_actions);
	basic_actions = NULL;
	sdm_object_deregister_type(&sdm_basic_obj_type);
}


int sdm_basic_read_entry(struct sdm_basic *basic, const char *name, struct sdm_data_file *data)
{
	char buffer[STRING_SIZE];

	if (sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		return(-1);
	if (!(basic->entry = sdm_hash_find_entry(basic_actions, buffer)))
		return(-1);
	SDM_ACTION(basic)->func = (sdm_action_t) basic->entry->data;
	return(SDM_HANDLED_ALL);
}

int sdm_basic_write_data(struct sdm_basic *basic, struct sdm_data_file *data)
{
	sdm_data_write_raw_string(data, basic->entry->name);
	return(0);
}

/*** Action Functions ***/

/**
 * Send a message to a user's output device.
 *	caller:		object that is telling the user something
 *	thing:		the user to tell
 *	target:		not used
 *	args:		the text to output
 */
int sdm_basic_action_tell_user(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	if (!sdm_object_is_a(SDM_OBJECT(thing), &sdm_user_obj_type) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args));
}

/**
 * Send a message to all children of a container.
 *	caller:		object that is announcing
 *	thing:		the container
 *	target:		not used
 *	args:		the text to output
 */
int sdm_basic_action_announce(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	struct sdm_thing *cur;

	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur != caller)
			sdm_thing_do_action(cur, caller, "tell", NULL, args, NULL);
	}
	return(0);
}

/**
 * Send a message to all everyone in the same room as the thing.
 *	caller:		object that is saying something
 *	thing:		the room in which something is being said
 *	target:		not used
 *	args:		the text to output
 */
int sdm_basic_action_say(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	const char *name;

	if (!args || (*args == '\0'))
		return(-1);
	sdm_thing_format_action(caller, caller, "tell", "You say \"%s\"\n", args);
	name = sdm_thing_get_string_property(caller, "name");
	sdm_thing_format_action(thing, caller, "announce", "\n%s says \"%s\"\n", name ? name : "something", args);
	return(0);
}

/**
 * Perform the 'examine' action on the target object or on the object itself if no target is given.
 *	caller:		object that is looking
 *	thing:		the room in which the caller is located
 *	target:		the object being looked at
 *	args:		the name of the object being looked at if target is NULL, unused otherwise
 */
int sdm_basic_action_look(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	if (!target && args && (*args != '\0')) {
		target = sdm_interpreter_get_thing(caller, args, NULL);
		if (!target) {
			sdm_thing_format_action(caller, caller, "tell", "You don't see a %s here\n", args);
			return(0);
		}
	}
	if (!target)
		target = thing;
	sdm_thing_do_action(target, caller, "examine", NULL, "", NULL);
	return(0);

}

/**
 * Print a description of a thing and list it's contents if it's a container
 *	caller:		object that is examining
 *	thing:		the object being examined
 *	target:		not used
 *	args:		not used
 */
int sdm_basic_action_examine(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	const char *str;
	struct sdm_thing *cur;

	if ((str = sdm_thing_get_string_property(thing, "name")))
		sdm_thing_format_action(caller, caller, "tell", "<brightyellow>%s</brightyellow>\n", str);
	if ((str = sdm_thing_get_string_property(thing, "description")))
		sdm_thing_format_action(caller, caller, "tell", "<brightgreen>%s</brightgreen>\n", str);
	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur == caller)
			continue;
		if (!(str = sdm_thing_get_string_property(cur, "name")))
			continue;
		//if (sdm_thing_is_a(cur, exit)) {
			// TODO print properly
		//}
		//else
			sdm_thing_format_action(caller, caller, "tell", "<brightblue>You see %s here.</brightblue>\n", str);
	}
	return(0);
}

/**
 * Move the caller to the target of thing.
 *	caller:		object to be moved
 *	thing:		the object containing the target of the move
 *	target:		not used
 *	args:		not used
 */
int sdm_basic_action_move(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	double num;
	struct sdm_thing *location;

	if (((num = sdm_thing_get_number_property(thing, "target")) <= 0)
	    || !(location = sdm_thing_lookup_id((sdm_id_t) num)))
		return(-1);
	sdm_thing_add(location, caller);
	return(0);
}

/**
 * Create a basic object given a parent object and an optional name
 *	caller:		creator of the object
 *	thing:		not used
 *	target:		not used
 *	args:		not used
 */
int sdm_basic_action_create_object(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{
	if (!target && args && (*args != '\0')) {
		target = sdm_interpreter_get_thing(caller, args, NULL);
		if (!target) {
			sdm_thing_format_action(caller, caller, "tell", "<red>Unable to determine the parent\n");
			return(0);
		}
	}


}

int sdm_basic_action_create_room(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{

}

int sdm_basic_action_create_exit(struct sdm_action *action, struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *target, const char *args, struct sdm_object **result)
{

}

