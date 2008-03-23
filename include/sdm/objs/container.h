/*
 * Object Name:	container.h
 * Description:	Actionable Container Object
 */

#ifndef _SDM_OBJS_CONTAINER_H
#define _SDM_OBJS_CONTAINER_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

#define SDM_CONTAINER(ptr)	( (struct sdm_container *) (ptr) )

struct sdm_container {
	struct sdm_thing thing;
	struct sdm_thing *objects;
	struct sdm_thing *end_objects;
};

#define SDM_CONTAINER_ARGS(id, parent)		SDM_THING_ARGS((id), (parent))

extern struct sdm_object_type sdm_container_obj_type;

int sdm_container_init(struct sdm_container *, va_list);
void sdm_container_release(struct sdm_container *);
int sdm_container_read_entry(struct sdm_container *, const char *, struct sdm_data_file *);
int sdm_container_write_data(struct sdm_container *, struct sdm_data_file *);

int sdm_container_add(struct sdm_container *, struct sdm_thing *);
int sdm_container_remove(struct sdm_container *, struct sdm_thing *);

#endif

