/*
 * Name:	funcs.c
 * Description:	SDRL Functions
 */

#include <sdrl/sdrl.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/objs/number.h>

#include <sdm/modules/sdrl/funcs.h>
#include <sdm/modules/sdrl/object.h>

int sdm_load_sdrl_library(struct sdrl_machine *mach)
{
	struct sdrl_type *func;

	if (!(func = sdrl_find_binding(mach->type_env, "func")))
		return(-1);

	/** Bind functions */
	SDRL_BIND_FUNCTION(mach, func, "do_action", sdm_sdrl_do_action);
	SDRL_BIND_FUNCTION(mach, func, "get_thing", sdm_sdrl_get_thing);
	SDRL_BIND_FUNCTION(mach, func, "get_location", sdm_sdrl_get_location);
	SDRL_BIND_FUNCTION(mach, func, "get_parent", sdm_sdrl_get_parent);
	SDRL_BIND_FUNCTION(mach, func, "get_property", sdm_sdrl_get_property);
	SDRL_BIND_FUNCTION(mach, func, "set_property", sdm_sdrl_set_property);

	SDRL_BIND_FUNCTION(mach, func, "add_thing", sdm_sdrl_add_thing);

	SDRL_BIND_FUNCTION(mach, func, "is_user", sdm_sdrl_is_user);
	return(0);
}

/**
 * Args:	<thing>, <caller, <action>, [<target>], [<args>]
 * Description:	Calls the given action name on the given object reference.
 */
int sdm_sdrl_do_action(struct sdrl_machine *mach, struct sdrl_value *args)
{
	int res;
	struct sdrl_type *type;
	const char *action, *rest;
	struct sdm_thing *caller, *thing, *target;

	thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	caller = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	action = sdm_sdrl_get_string(&args);
	target = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	rest = sdm_sdrl_get_string(&args);

	if (!caller || !thing || !action)
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));
	res = sdm_thing_do_action(thing, caller, action, target, rest);
	if (!(type = sdrl_find_binding(mach->type_env, "number")))
		return(SDRL_ERROR(mach, SDRL_ES_HIGH, SDRL_ERR_NOT_FOUND, NULL));
	mach->ret = sdrl_make_number(mach->heap, type, res);
	return(0);
}

/**
 * Args:	<id>
 * Description:	Returns the thing with the given id.
 */
int sdm_sdrl_get_thing(struct sdrl_machine *mach, struct sdrl_value *args)
{
	sdm_id_t id;
	struct sdm_thing *thing;

	id = sdm_sdrl_get_number(&args);
	thing = sdm_thing_lookup_id(id);
	if (thing)
		mach->ret = sdm_sdrl_reference_object(mach, SDM_OBJECT(thing));
	return(0);
}

/**
 * Args:	<thing>
 * Description:	Returns the location of the given thing.
 */
int sdm_sdrl_get_location(struct sdrl_machine *mach, struct sdrl_value *args)
{
	struct sdm_thing *thing;

	if (!(thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type))))
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));
	if (thing->location)
		mach->ret = sdm_sdrl_reference_object(mach, SDM_OBJECT(thing->location));
	return(0);
}

/**
 * Args:	<thing>
 * Description:	Returns the parent of the given thing.
 */
int sdm_sdrl_get_parent(struct sdrl_machine *mach, struct sdrl_value *args)
{
	struct sdm_thing *thing;

	if (!(thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type))))
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));
	if ((thing = sdm_thing_lookup_id(thing->parent)))
		mach->ret = sdm_sdrl_reference_object(mach, SDM_OBJECT(thing));
	return(0);
}

/**
 * Args:	<thing>, <name>
 * Description:	Returns the named property for the given thing
 */
int sdm_sdrl_get_property(struct sdrl_machine *mach, struct sdrl_value *args)
{
	const char *name;
	struct sdm_object *obj;
	struct sdm_thing *thing;

	thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	name = sdm_sdrl_get_string(&args);
	if (!thing || !name)
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));

	obj = sdm_thing_get_property(thing, name, NULL);
	if (sdm_object_is_a(obj, &sdm_number_obj_type))
		mach->ret = sdrl_make_number(mach->heap, sdrl_find_binding(mach->type_env, "number"), SDM_NUMBER(obj)->num);
	else if (sdm_object_is_a(obj, &sdm_string_obj_type))
		mach->ret = sdrl_make_string(mach->heap, sdrl_find_binding(mach->type_env, "string"), SDM_STRING(obj)->str, SDM_STRING(obj)->len);
	else
		mach->ret = sdm_sdrl_reference_object(mach, SDM_OBJECT(obj));
	return(0);
}

/**
 * Args:	<thing>, <name>, <object>
 * Description:	Sets the named property for the thing to the given object.
 */
int sdm_sdrl_set_property(struct sdrl_machine *mach, struct sdrl_value *args)
{
	int res;
	const char *name;
	struct sdm_object *obj;
	struct sdm_thing *thing;

	thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	name = sdm_sdrl_get_string(&args);
	if (!thing || !name || !args)
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));

	if (args->type->basetype == SDRL_BT_NUMBER)
		obj = SDM_OBJECT(create_sdm_number(SDRL_NUMBER(args)->num));
	else if (args->type->basetype == SDRL_BT_STRING)
		obj = SDM_OBJECT(create_sdm_string(SDRL_STRING(args)->str));
	else if (args->type->basetype != SDM_BT_SDM_OBJ_REF)
		obj = SDM_SDRL_OBJ_REF(args)->ref;
	else
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));

	res = sdm_thing_set_property(thing, name, obj);
	mach->ret = sdrl_make_number(mach->heap, sdrl_find_binding(mach->type_env, "number"), res);
	return(0);
}

/**
 * Args:	<container>, <thing>
 * Description:	Adds the given thing to the given container
 */
int sdm_sdrl_add_thing(struct sdrl_machine *mach, struct sdrl_value *args)
{
	int res;
	struct sdm_thing *thing, *container;

	container = sdm_sdrl_get_object(&args, &sdm_thing_obj_type);
	thing = SDM_THING(sdm_sdrl_get_object(&args, &sdm_thing_obj_type));
	if (!container || !thing)
		return(SDRL_ERROR(mach, SDRL_ES_LOW, SDRL_ERR_INVALID_ARGS, NULL));

	res = sdm_thing_add(thing, thing);
	mach->ret = sdrl_make_number(mach->heap, sdrl_find_binding(mach->type_env, "number"), res);
	return(0);
}

/**
 * Args:	<thing>
 * Description:	Returns 1 if the given object is an sdm_user or 0 otherwise.
 */
int sdm_sdrl_is_user(struct sdrl_machine *mach, struct sdrl_value *args)
{
	struct sdrl_type *type;

	if (!(type = sdrl_find_binding(mach->type_env, "number")))
		return(SDRL_ERROR(mach, SDRL_ES_HIGH, SDRL_ERR_NOT_FOUND, NULL));

	if (sdm_sdrl_get_object(&args, &sdm_user_obj_type))
		mach->ret = sdrl_make_number(mach->heap, type, 1);
	else
		mach->ret = sdrl_make_number(mach->heap, type, 0);
	return(0);
}


