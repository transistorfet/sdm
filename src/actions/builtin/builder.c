/*
 * Name:	builder.c
 * Description:	Builder Actions
 */

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/utils.h>
#include <sdm/processors/interpreter.h>

#include <sdm/actions/builtin/builder.h>

int sdm_builtin_load_builder(struct sdm_hash *actions)
{
	sdm_hash_add(actions, "builtin_create_object", sdm_builtin_action_create_object);
	sdm_hash_add(actions, "builtin_create_room", sdm_builtin_action_create_room);
	sdm_hash_add(actions, "builtin_create_exit", sdm_builtin_action_create_exit);
	return(0);
}

/**
 * Create a builtin object given a parent object and an optional name
 *	caller:		creator of the object
 *	thing:		not used
 *	target:		not used
 *	args:		not used
 */
int sdm_builtin_action_create_object(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;

	if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_do_format_action(args->caller, args->caller, "notify", "<red>Unable to find the given parent.\n");
		return(0);
	}

	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(args->obj)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, args->obj->id))))) {
		sdm_notify(args->caller, args->caller, "<red>Error creating object.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	sdm_moveto(obj, args->caller, NULL);
	sdm_notify(args->caller, args->caller, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

int sdm_builtin_action_create_room(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;
	struct sdm_thing *room;

	if (!(room = sdm_interpreter_find_thing(NULL, "/core/room")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(room)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, room->id))))) {
		sdm_notify(args->caller, args->caller, "<red>Error creating room.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	// TODO this is a dangerous dereference
	sdm_moveto(obj, args->caller->location->location, NULL);
	sdm_notify(args->caller, args->caller, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

int sdm_builtin_action_create_exit(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;
	struct sdm_thing *exit;

	if (!(exit = sdm_interpreter_find_thing(NULL, "/core/exit")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(exit)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, exit->id))))) {
		sdm_notify(args->caller, args->caller, "<red>Error creating exit.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	// TODO this is a somewhat dangerous dereference
	sdm_moveto(obj, args->caller->location, NULL);
	sdm_notify(args->caller, args->caller, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}


