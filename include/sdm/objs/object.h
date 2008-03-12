/*
 * Object Name:	object.h
 * Description:	Base Object
 */

#ifndef _SDM_OBJS_OBJECT_H
#define _SDM_OBJS_OBJECT_H

#include <stdarg.h>

#define SDM_OBF_RELEASING	0x1000

#define SDM_OBJECT(ptr)		( (struct sdm_object *) (ptr) )
#define SDM_OBJECT_TYPE(ptr)	( (struct sdm_object_type *) (ptr) )

struct sdm_object;
struct sdm_object_type;

typedef int (*sdm_object_init_t)(struct sdm_object *, va_list);
typedef void (*sdm_object_release_t)(struct sdm_object *);

struct sdm_object {
	struct sdm_object_type *type;
	int bitflags;

};

struct sdm_object_type {
	struct sdm_object_type *parent;
	int size;
	void *ptr;
	sdm_object_init_t init;
	sdm_object_release_t release;
};

struct sdm_object *create_sdm_object(struct sdm_object_type *, ...);
void destroy_sdm_object(struct sdm_object *);

static inline int SDM_OBJECT_IS_A(struct sdm_object *obj, struct sdm_object_type *type) {
	struct sdm_object_type *cur;

	for (cur = obj->type; cur; cur = cur->parent) {
		if (cur == type)
			return(1);
	}
	return(0);
}

static inline int CALL_SDM_OBJECT_INIT(sdm_object_init_t init, struct sdm_object *obj, ...) {
	int ret;
	va_list va;

	va_start(va, obj);
	ret = init(obj, va);
	va_end(va);
	return(ret);
}

#endif

