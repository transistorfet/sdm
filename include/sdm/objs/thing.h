/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_OBJS_THING_H
#define _SDM_OBJS_THING_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/objs/object.h>

#define SDM_THING(ptr)	( (struct sdm_thing *) (ptr) )

struct sdm_user;
struct sdm_container;
struct sdm_thing;

typedef int (*sdm_action_t)(void *, struct sdm_user *, struct sdm_thing *, const char *);

struct sdm_thing {
	struct sdm_object object;
	int id;
	struct sdm_thing *parent;
	struct sdm_container *owner;
	struct sdm_thing *next;
	struct sdm_hash *properties;
	struct sdm_hash *actions;

};

#define SDM_THING_ARGS(id, parent)		(id), (parent)

extern struct sdm_object_type sdm_thing_obj_type;

int init_thing(void);
int release_thing(void);

int sdm_thing_init(struct sdm_thing *, va_list);
void sdm_thing_release(struct sdm_thing *);
int sdm_thing_read_entry(struct sdm_thing *, const char *, struct sdm_data_file *);
int sdm_thing_write_data(struct sdm_thing *, struct sdm_data_file *);

int sdm_thing_set_property(struct sdm_thing *, const char *, struct sdm_object *);
struct sdm_object *sdm_thing_get_property(struct sdm_thing *, const char *, struct sdm_object_type *);

int sdm_thing_set_action(struct sdm_thing *, const char *, sdm_action_t, void *, destroy_t);
int sdm_thing_do_action(struct sdm_thing *, struct sdm_user *, const char *, const char *);

int sdm_thing_assign_id(struct sdm_thing *, int);
int sdm_thing_assign_new_id(struct sdm_thing *);
struct sdm_thing *sdm_thing_lookup_id(int);

#endif

