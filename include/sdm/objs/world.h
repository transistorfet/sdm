/*
 * Object Name:	world.h
 * Description:	Root World Object
 */

#ifndef _SDM_OBJS_WORLD_H
#define _SDM_OBJS_WORLD_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>
#include <sdm/objs/container.h>

#define SDM_WORLD(ptr)		( (struct sdm_world *) (ptr) )

struct sdm_world {
	struct sdm_container container;
};

extern struct sdm_object_type sdm_world_obj_type;

int init_world(void);
int release_world(void);

int sdm_world_init(struct sdm_world *, va_list);
void sdm_world_release(struct sdm_world *);

struct sdm_world *sdm_world_get_root(void);

#endif

