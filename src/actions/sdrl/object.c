/*
 * Type Name:		env.c
 * Module Requirements:	(none)
 * Description:		SDM Object Type
 */

#include <sdrl/sdrl.h>

#include <sdm/objs/object.h>
#include <sdm/actions/sdrl/object.h>

struct sdrl_type *sdm_sdrl_make_object_type(struct sdrl_machine *mach)
{
	return(sdrl_make_type(
		sizeof(struct sdm_sdrl_obj_ref),
		0,
		SDM_BT_SDM_OBJ_REF,
		NULL,
		(sdrl_destroy_t) sdrl_heap_free,
		(sdrl_duplicate_t) sdm_sdrl_duplicate_obj_ref,
		NULL
	));
}

struct sdrl_value *sdm_sdrl_make_obj_ref(struct sdrl_heap *heap, struct sdrl_type *type, struct sdm_object *ref)
{
	struct sdm_sdrl_obj_ref *value;

	if (!(value = (struct sdm_sdrl_obj_ref *) sdrl_heap_alloc(heap, type->size)))
		return(NULL);
	SDRL_VALUE(value)->refs = 1;
	SDRL_VALUE(value)->type = type;
	SDRL_VALUE(value)->next = NULL;
	value->ref = ref;
	return(SDRL_VALUE(value));
}

struct sdrl_value *sdm_sdrl_duplicate_obj_ref(struct sdrl_machine *mach, struct sdm_sdrl_obj_ref *org)
{
	return(sdm_sdrl_make_obj_ref(mach->heap, SDRL_VALUE(org)->type, org->ref));
}


