/*
 * Header Name:	object.h
 * Description:	SDM Object Type Header
 */

#ifndef _SDM_ACTIONS_SDRL_OBJECT_H
#define _SDM_ACTIONS_SDRL_OBJECT_H

#include <sdrl/sdrl.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define SDM_BT_SDM_OBJ_REF		( SDRL_BT_USER_BASETYPE + 1 )

#define SDM_SDRL_OBJ_REF(ptr)		( (struct sdm_sdrl_obj_ref *) (ptr) )

struct sdm_sdrl_obj_ref {
	struct sdrl_value value;
	struct sdm_object *ref;
};

struct sdrl_type *sdm_sdrl_make_object_type(struct sdrl_machine *);
struct sdrl_value *sdm_sdrl_make_obj_ref(struct sdrl_heap *, struct sdrl_type *, struct sdm_object *);
struct sdrl_value *sdm_sdrl_duplicate_obj_ref(struct sdrl_machine *, struct sdm_sdrl_obj_ref *);

static inline struct sdrl_value *sdm_sdrl_reference_object(struct sdrl_machine *mach, struct sdm_object *obj) {
	struct sdrl_type *type;

	if (!(type = sdrl_find_binding(mach->type_env, "object")))
		return(NULL);
	return(sdm_sdrl_make_obj_ref(mach->heap, type, obj));
}

#endif

