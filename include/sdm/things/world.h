/*
 * Object Name:	world.h
 * Description:	Root World Object
 */

#ifndef _SDM_OBJS_WORLD_H
#define _SDM_OBJS_WORLD_H

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

#define SDM_WORLD(ptr)		( (struct sdm_world *) (ptr) )

struct sdm_world {
	struct sdm_thing thing;
	string_t filename;
};

#define SDM_WORLD_ARGS(file, id, parent)		(file), SDM_THING_ARGS((id), (parent))

extern struct sdm_object_type sdm_world_obj_type;

#define create_sdm_world(file, id, parent)	\
	( create_sdm_object(&sdm_world_obj_type, 3, SDM_WORLD_ARGS((file), (id), (parent))) )

int init_world(void);
void release_world(void);

int sdm_world_init(struct sdm_world *, int, va_list);
void sdm_world_release(struct sdm_world *);
int sdm_world_read_entry(struct sdm_world *, const char *, struct sdm_data_file *);

int sdm_world_write(struct sdm_world *, struct sdm_data_file *);

struct sdm_world *sdm_world_get_root(void);

#endif

