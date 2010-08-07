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
#include <sdm/tasks/interpreter.h>

static int builder_create(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int builder_create_room(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int builder_add_exit(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
static int builder_set(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

int sdm_builtin_load_builder(struct sdm_hash *actions)
{
	sdm_hash_add(actions, "builder_create", builder_create);
	sdm_hash_add(actions, "builder_create_room", builder_create_room);
	sdm_hash_add(actions, "builder_add_exit", builder_add_exit);
	sdm_hash_add(actions, "builder_set", builder_set);
	return(0);
}

/**
 * Create a builtin object given a parent object and an optional name
 *	caller:		creator of the object
 *	thing:		not used
 *	target:		not used
 *	args:		not used
 */
static int builder_create(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;

	if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "<red>Unable to find the given parent.\n");
		return(0);
	}

	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(args->obj)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, args->obj->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating object.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	sdm_moveto(args->caller, obj, args->caller, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

static int builder_create_room(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	struct sdm_thing *obj;
	struct sdm_thing *room;

	if (!(room = sdm_interpreter_find_thing(NULL, "/core/room")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(room)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, room->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating room.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	// TODO this is a dangerous dereference
	sdm_moveto(args->caller, obj, args->caller->location->location, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

static int builder_add_exit(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	int i = 0;
	char buffer[STRING_SIZE];
	struct sdm_thing *obj, *exit, *target;

	if (!(exit = sdm_interpreter_find_thing(NULL, "/core/exit")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(exit)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, exit->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating exit.\n");
		return(-1);
	}

	sdm_interpreter_get_string(args->text, buffer, STRING_SIZE, &i);
	sdm_set_string_property(obj, "name", buffer);
	if ((target = sdm_interpreter_get_thing(args->caller, &args->text[i], &i)))
		sdm_set_number_property(obj, "target", target->id);

	// TODO this is a somewhat dangerous dereference
	sdm_moveto(args->caller, obj, args->caller->location, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
	return(0);
}

static int builder_set(struct sdm_action *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	int i = 0;
	char prop[STRING_SIZE];
	struct sdm_string *string;

	if (sdm_interpreter_get_string(args->text, prop, STRING_SIZE, &i) <= 0) {
		sdm_notify(args->caller, args, "<red>Invalid property.\n");
		return(-1);
	}
	if (!args->obj && !(args->obj = sdm_interpreter_get_thing(args->caller, &args->text[i], &i))) {
		sdm_notify(args->caller, args, "<red>Object not found.\n");
		return(-1);
	}

	if (!(string = create_sdm_string(&args->text[i])) || (sdm_thing_set_property(args->obj, prop, SDM_OBJECT(string)) >= 0))
		sdm_notify(args->caller, args, "<green>Property set successfully.\n");
	else
		sdm_notify(args->caller, args, "<red>Error setting property.\n");
	return(0);
}


