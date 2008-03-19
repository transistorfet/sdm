/*
 * Object Name:	world.h
 * Description:	Root World Object
 */

#ifndef _SDM_OBJS_WORLD_H
#define _SDM_OBJS_WORLD_H

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/string.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/container.h>

#define SDM_WORLD(ptr)		( (struct sdm_world *) (ptr) )

struct sdm_world {
	struct sdm_container container;
	string_t filename;
};

#define SDM_WORLD_ARGS(file, id, parent)		(file), SDM_CONTAINER_ARGS((id), (parent))

extern struct sdm_object_type sdm_world_obj_type;

#define create_sdm_world(file, id, parent)	\
	( create_sdm_object(&sdm_world_obj_type, SDM_WORLD_ARGS((file), (id), (parent))) )

int init_world(void);
void release_world(void);

int sdm_world_init(struct sdm_world *, va_list);
void sdm_world_release(struct sdm_world *);
int sdm_world_read_entry(struct sdm_world *, const char *, struct sdm_data_file *);

int sdm_world_write(struct sdm_world *, struct sdm_data_file *);

struct sdm_world *sdm_world_get_root(void);

#endif

