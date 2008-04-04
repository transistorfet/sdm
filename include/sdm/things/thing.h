/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_THINGS_THING_H
#define _SDM_THINGS_THING_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/tree.h>
#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/actions/action.h>

#define SDM_NO_ID		-1
#define SDM_NEW_ID		-2

typedef int sdm_id_t;

#define SDM_THING(ptr)	( (struct sdm_thing *) (ptr) )

struct sdm_thing {
	struct sdm_object object;
	sdm_id_t id;
	sdm_id_t parent;
	struct sdm_thing *location;
	struct sdm_hash *properties;
	struct sdm_tree *actions;

	struct sdm_thing *next;
	struct sdm_thing *objects;
	struct sdm_thing *end_objects;
};

#define SDM_THING_ARGS(id, parent)		(id), (parent)

extern struct sdm_object_type sdm_thing_obj_type;

int init_thing(void);
void release_thing(void);

int sdm_thing_init(struct sdm_thing *, int, va_list);
void sdm_thing_release(struct sdm_thing *);
int sdm_thing_read_entry(struct sdm_thing *, const char *, struct sdm_data_file *);
int sdm_thing_write_data(struct sdm_thing *, struct sdm_data_file *);

int sdm_thing_set_property(struct sdm_thing *, const char *, struct sdm_object *);
struct sdm_object *sdm_thing_get_property(struct sdm_thing *, const char *, struct sdm_object_type *);

int sdm_thing_set_action(struct sdm_thing *, const char *, struct sdm_action *);
int sdm_thing_do_action(struct sdm_thing *, const char *, struct sdm_action_args *);

int sdm_thing_add(struct sdm_thing *, struct sdm_thing *);
int sdm_thing_remove(struct sdm_thing *, struct sdm_thing *);

int sdm_thing_assign_id(struct sdm_thing *, sdm_id_t);
int sdm_thing_assign_new_id(struct sdm_thing *);


/*** Thing ID Functions ***/

extern int sdm_thing_table_size;
extern struct sdm_thing **sdm_thing_table;

static inline struct sdm_thing *sdm_thing_lookup_id(sdm_id_t id) {
	if ((id >= 0) && (id < sdm_thing_table_size))
		return(sdm_thing_table[id]);
	return(NULL);
}

static inline int sdm_thing_is_a(struct sdm_thing *thing, sdm_id_t id) {
	for (; thing; thing = sdm_thing_lookup_id(thing->parent)) {
		if (thing->id == id)
			return(1);
	}
	return(0);
}

#endif

