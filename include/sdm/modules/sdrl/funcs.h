/*
 * Header Name:	funcs.h
 * Description:	SDRL Functions Header
 */

#ifndef _SDM_MODULES_SDRL_FUNCS_H
#define _SDM_MODULES_SDRL_FUNCS_H

#include <sdrl/sdrl.h>

#include <sdm/globals.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>

#include <sdm/modules/sdrl/object.h>


int sdm_load_sdrl_library(struct sdrl_machine *);

int sdm_sdrl_do_action(struct sdrl_machine *, struct sdrl_value *);

static inline const char *sdm_sdrl_get_string(struct sdrl_value **args)
{
	struct sdrl_value *value;

	if (!args || !(value = *args))
		return(NULL);
	*args = (*args)->next;
	if (value->type->basetype != SDRL_BT_STRING)
		return(NULL);
	return(SDRL_STRING(value)->str);
}

static inline struct sdm_object *sdm_sdrl_get_object(struct sdrl_value **args, struct sdm_object_type *type)
{
	struct sdrl_value *value;

	if (!args || !(value = *args))
		return(NULL);
	*args = (*args)->next;
	if (value->type->basetype != SDM_BT_SDM_OBJ_REF)
		return(NULL);
	if (type && (sdm_object_is_a(SDM_SDRL_OBJ_REF(value)->ref, type)))
		return(NULL);
	return(SDM_SDRL_OBJ_REF(value)->ref);
}

#endif


