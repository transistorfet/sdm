/*
 * Object Name:	mobile.h
 * Description:	Base Self-Movable Object
 */

#ifndef _SDM_OBJS_MOBILE_H
#define _SDM_OBJS_MOBILE_H

#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/container.h>

#define SDM_MOBILE(ptr)		( (struct sdm_mobile *) (ptr) )

struct sdm_mobile {
	struct sdm_container container;

};

extern struct sdm_object_type sdm_mobile_obj_type;

int sdm_mobile_init(struct sdm_mobile *, va_list);
void sdm_mobile_release(struct sdm_mobile *);

#endif

