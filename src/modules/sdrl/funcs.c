/*
 * Name:	funcs.c
 * Description:	SDRL Functions
 */

#include <sdrl/sdrl.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>

#include <sdm/modules/sdrl/funcs.h>
#include <sdm/modules/sdrl/object.h>

int sdm_load_sdrl_library(struct sdrl_machine *mach)
{
	struct sdrl_type *func;

	if (!(func = sdrl_find_binding(mach->type_env, "func")))
		return(-1);

	/** Bind functions */
	SDRL_BIND_FUNCTION(mach, func, "do_action", sdm_sdrl_do_action);
	return(0);
}

/**
 * Args:	<value>, ...
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
		return(-1);
	res = sdm_thing_do_action(thing, caller, action, target, rest);
	if (!(type = sdrl_find_binding(mach->type_env, "number")))
		return(SDRL_ERROR(mach, SDRL_ES_HIGH, SDRL_ERR_NOT_FOUND, NULL));
	mach->ret = sdrl_make_number(mach->heap, type, res);
	return(0);
}


