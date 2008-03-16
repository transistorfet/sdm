/*
 * Name:	sdrl.c
 * Description:	SDRL Scripting Module
 */

#include <stdio.h>
#include <string.h>
#include <sdrl/sdrl.h>
#include <sdrl/lib/base.h>

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
#include <sdm/objs/container.h>
#include <sdm/modules/module.h>
#include <sdm/modules/sdrl/sdrl.h>

struct sdm_object_type sdm_sdrl_obj_type = {
	&sdm_action_obj_type,
	sizeof(struct sdm_sdrl),
	NULL,
	(sdm_object_init_t) NULL,
	(sdm_object_release_t) NULL,
	(sdm_object_read_entry_t) sdm_sdrl_read_entry,
	(sdm_object_write_data_t) sdm_sdrl_write_data
};

static struct sdrl_machine *global_mach = NULL;

static int sdm_load_sdrl_library(struct sdrl_machine *);

int init_sdrl(void)
{
	if (global_mach)
		return(1);
	if (!(global_mach = sdrl_create_machine()))
		return(-1);
	if (sdrl_load_base(global_mach))
		return(-1);
	if (sdm_load_sdrl_library(global_mach))
		return(-1);
	if (sdm_object_register_type("sdrl", &sdm_sdrl_obj_type) < 0)
		return(-1);
	return(0);
}

int release_sdrl(void)
{
	if (!global_mach)
		return(1);
	sdm_object_deregister_type("sdrl");
	if (global_mach)
		sdrl_destroy_machine(global_mach);
	global_mach = NULL;
	return(0);
}


int sdm_sdrl_read_entry(struct sdm_sdrl *action, const char *name, struct sdm_data_file *data)
{
	int res;
	struct sdrl_expr *expr;
	char buffer[LARGE_STRING_SIZE];

	if ((res = sdm_data_read_string(data, buffer, LARGE_STRING_SIZE)) < 0)
		return(-1);
	if (!(expr = sdrl_base_parse_string(global_mach, (sdrl_parser_t) sdrl_base_parse_lambda_input, buffer, res)))
		return(-1);
	SDM_ACTION(action)->func = (sdm_action_t) sdm_sdrl_do_action;
	action->expr = expr;
	return(SDM_HANDLED);
}

int sdm_sdrl_write_data(struct sdm_sdrl *action, struct sdm_data_file *data)
{
	// TODO implement
	return(0);
}


int sdm_sdrl_do_action(struct sdrl_expr *expr, struct sdm_user *user, struct sdm_thing *thing, const char *args)
{
	if (!(global_mach->env = sdrl_extend_environment(global_mach->env))) {
		SDRL_ERROR(global_mach, SDRL_ES_HIGH, SDRL_ERR_OUT_OF_MEMORY, NULL);
		return(-1);
	}
	// TODO add bindings for user, this, and args
	//sdrl_add_binding(env, "args", SDRL_MAKE_REFERENCE(args));
	sdrl_evaluate(global_mach, expr);
	global_mach->env = sdrl_retract_environment(global_mach->env);
	return(0);
}

static int sdm_load_sdrl_library(struct sdrl_machine *mach)
{
	return(0);
}


