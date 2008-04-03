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

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/utils.h>
#include <sdm/processors/interpreter.h>

#include <sdm/actions/action.h>
#include <sdm/actions/basic/basic.h>

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
	if (!(basic_actions = create_sdm_hash(0, -1, NULL)))
		return(-1);
	if (sdm_object_register_type(&sdm_basic_obj_type) < 0)
		return(-1);

	sdm_hash_add(basic_actions, "basic_notify_user", sdm_basic_action_notify_user);
	sdm_hash_add(basic_actions, "basic_announce", sdm_basic_action_announce);
	sdm_hash_add(basic_actions, "basic_say", sdm_basic_action_say);
	sdm_hash_add(basic_actions, "basic_look", sdm_basic_action_look);
	sdm_hash_add(basic_actions, "basic_examine", sdm_basic_action_examine);
	sdm_hash_add(basic_actions, "basic_move", sdm_basic_action_move);

	sdm_hash_add(basic_actions, "basic_inventory", sdm_basic_action_inventory);
	sdm_hash_add(basic_actions, "basic_get", sdm_basic_action_get);
	sdm_hash_add(basic_actions, "basic_drop", sdm_basic_action_drop);

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
 *	caller:		object that is notifying the user something
 *	thing:		the user to notify
 *	target:		not used
 *	args:		the text to output
 */
int sdm_basic_action_notify_user(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (!sdm_object_is_a(SDM_OBJECT(thing), &sdm_user_obj_type) || !SDM_USER(thing)->inter)
		return(-1);
	return(SDM_INTERFACE_WRITE(SDM_USER(thing)->inter, args->text));
}

/**
 * Send a message to all children of a container.
 *	caller:		object that is announcing
 *	thing:		the container
 *	target:		not used
 *	args:		the text to output
 */
int sdm_basic_action_announce(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *cur;

	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur != args->caller)
			sdm_thing_do_format_action(cur, args->caller, "notify", "%s", args->text);
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
int sdm_basic_action_say(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *name;

	if (*args->text == '\0')
		return(-1);
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "You say \"%s\"\n", args->text);
	name = sdm_thing_get_string_property(args->caller, "name");
	sdm_thing_do_format_action(thing, args->caller, "announce", "\n%s says \"%s\"\n", name ? name : "something", args);
	return(0);
}

/**
 * Perform the 'examine' action on the target object or on the object itself if no target is given.
 *	caller:		object that is looking
 *	thing:		the room in which the caller is located
 *	target:		the object being looked at
 *	args:		the name of the object being looked at if target is NULL, unused otherwise
 */
int sdm_basic_action_look(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	if (*args->text == '\0')
		args->obj = thing;
	else if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "You don't see a %s here\n", args->text);
		return(0);
	}
	sdm_thing_do_nil_action(args->obj, args->caller, "examine");
	return(0);

}

/**
 * Print a description of a thing and list it's contents if it's a container
 *	caller:		object that is examining
 *	thing:		the object being examined
 *	target:		not used
 *	args:		not used
 */
int sdm_basic_action_examine(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *str;
	struct sdm_thing *cur;

	if ((str = sdm_thing_get_string_property(thing, "name")))
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightyellow>%s</brightyellow>\n", str);
	if ((str = sdm_thing_get_string_property(thing, "description")))
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightgreen>%s</brightgreen>\n", str);
	for (cur = thing->objects; cur; cur = cur->next) {
		if (cur == args->caller)
			continue;
		sdm_thing_do_nil_action(cur, args->caller, "tell_view");

/*
		if (!(str = sdm_thing_get_string_property(cur, "name")))
			continue;
		//if (sdm_thing_is_a(cur, exit)) {
			// TODO print properly
		//}
		//else
			sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightblue>You see %s here.</brightblue>\n", str);
*/
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
int sdm_basic_action_move(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	double num;
	struct sdm_thing *location;

	if (((num = sdm_thing_get_number_property(thing, "target")) <= 0)
	    || !(location = sdm_thing_lookup_id((sdm_id_t) num)))
		return(-1);
	sdm_moveto(args->caller, location, NULL);
	return(0);
}


int sdm_basic_action_inventory(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *str;
	struct sdm_thing *cur;

	if (!thing->objects) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightgreen>You aren't carrying anything.\n");
		return(0);
	}

	sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightgreen>You are carrying:\n");
	for (cur = thing->objects; cur; cur = cur->next) {
		if (!(str = sdm_thing_get_string_property(cur, "name")))
			continue;
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<brightblue>    %s.\n", str);
	}
	return(0);
}

int sdm_basic_action_get(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *name, *objname;

	if (sdm_interpreter_parse_args(args, 2) < 0) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "You don't see %s here\n", args->text);
		return(0);
	}
	if (args->obj->location == args->caller) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "You already have that.\n");
		return(0);
	}
	// TODO call action on object to see if it is gettable
	sdm_moveto(args->obj, args->caller, NULL);
	objname = sdm_thing_get_string_property(args->obj, "name");
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "You get %s.\n", objname ? objname : "something");
	name = sdm_thing_get_string_property(args->caller, "name");
	sdm_thing_do_format_action(args->caller->location, args->caller, "announce", "%s gets %s.\n", name ? name : "something", objname ? objname : "something");
	return(0);
}

int sdm_basic_action_drop(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	int i = 0;
	const char *name, *objname;

	if (!args->obj && ((*args->text == '\0')
	    || !(args->obj = sdm_interpreter_get_thing(args->caller, args->text, &i))
	    || (args->obj->location != args->caller))) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "You aren't carrying that.\n");
		return(-1);
	}
	// TODO call action on object to see if it is removable
	// TODO call action on room to see if object can be dropped here
	sdm_moveto(args->obj, args->caller->location, NULL);
	objname = sdm_thing_get_string_property(args->obj, "name");
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "You drop %s.\n", objname ? objname : "something");
	name = sdm_thing_get_string_property(args->caller, "name");
	sdm_thing_do_format_action(args->caller->location, args->caller, "announce", "%s drops %s.\n", name ? name : "something", objname ? objname : "something");
	return(0);
}


/**
 * Create a basic object given a parent object and an optional name
 *	caller:		creator of the object
 *	thing:		not used
 *	target:		not used
 *	args:		not used
 */
int sdm_basic_action_create_object(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;

	if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<red>Unable to find the given parent.\n");
		return(0);
	}

	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(args->obj)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, args->obj->id))))) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<red>Error creating object.\n");
		return(-1);
	}

	sdm_thing_set_string_property(obj, "name", args->text);
	sdm_moveto(obj, args->caller, NULL);
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

int sdm_basic_action_create_room(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;
	struct sdm_thing *room;

	if (!(room = sdm_interpreter_find_thing(NULL, "/core/room")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(room)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, room->id))))) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<red>Error creating room.\n");
		return(-1);
	}

	sdm_thing_set_string_property(obj, "name", args->text);
	// TODO this is a dangerous dereference
	sdm_moveto(obj, args->caller->location->location, NULL);
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

int sdm_basic_action_create_exit(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;
	struct sdm_thing *exit;

	if (!(exit = sdm_interpreter_find_thing(NULL, "/core/exit")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(exit)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, exit->id))))) {
		sdm_thing_do_format_action(args->caller, args->caller, "notify", "<red>Error creating exit.\n");
		return(-1);
	}

	sdm_thing_set_string_property(obj, "name", args->text);
	// TODO this is a somewhat dangerous dereference
	sdm_moveto(obj, args->caller->location, NULL);
	sdm_thing_do_format_action(args->caller, args->caller, "notify", "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

