/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>

#include <sdm/objs/object.h>

struct sdm_object *create_sdm_object(struct sdm_object_type *type, ...)
{
	va_list va;
	struct sdm_object *obj;

	if (!(obj = memory_alloc(type->size)))
		return(NULL);
	memset(obj, '\0', type->size);
	obj->type = type;

	va_start(va, type);
	if (type->init && type->init(obj, va)) {
		destroy_sdm_object(obj);
		return(NULL);
	}
	va_end(va);
	return(obj);
}

void destroy_sdm_object(struct sdm_object *obj)
{
	/** We don't want to attempt to free this object twice */
	if (!obj || (obj->bitflags & SDM_OBF_RELEASING))
		return;
	obj->bitflags |= SDM_OBF_RELEASING;
	if (obj->type->release)
		obj->type->release(obj);
	memory_free(obj);
}

