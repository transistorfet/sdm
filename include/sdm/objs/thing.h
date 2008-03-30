/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_OBJS_THING_H
#define _SDM_OBJS_THING_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/objs/action.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

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
	struct sdm_hash *actions;

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


/*** Thing Property Functions ***/

static inline double sdm_thing_get_number_property(struct sdm_thing *thing, const char *name) {
	struct sdm_number *number;

	if (!(number = SDM_NUMBER(sdm_thing_get_property(thing, name, &sdm_number_obj_type))))
		return(0);
	return(number->num);
}

static inline int sdm_thing_set_number_property(struct sdm_thing *thing, const char *name, double num) {
	struct sdm_number *number;

	if (!(number = create_sdm_number(num)))
		return(-1);
	return(sdm_thing_set_property(thing, name, SDM_OBJECT(number)));
}

static inline const char *sdm_thing_get_string_property(struct sdm_thing *thing, const char *name) {
	struct sdm_string *string;

	if (!(string = SDM_STRING(sdm_thing_get_property(thing, name, &sdm_string_obj_type))))
		return(NULL);
	return(string->str);
}

static inline int sdm_thing_set_string_property(struct sdm_thing *thing, const char *name, const char *str) {
	struct sdm_string *string;

	if (!(string = create_sdm_string(str)))
		return(-1);
	return(sdm_thing_set_property(thing, name, SDM_OBJECT(string)));
}


/*** Thing Action Functions ***/

static inline int sdm_thing_do_nil_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action) {
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.obj = thing;
	args.text = "";
	return(sdm_thing_do_action(thing, action, &args));
}

static inline int sdm_thing_do_format_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, const char *fmt, ...) {
	int i;
	va_list va;
	char buffer[STRING_SIZE];
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = buffer;

	va_start(va, fmt);
	if ((i = vsnprintf(buffer, STRING_SIZE - 1, fmt, va)) < 0)
		return(-1);
	if (i >= STRING_SIZE - 1)
		buffer[STRING_SIZE - 1] = '\0';
	sdm_thing_do_action(thing, action, &args);
	return(0);
}

static inline struct sdm_object *sdm_thing_get_action_object(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, struct sdm_object_type *type) {
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = "";

	sdm_thing_do_action(thing, action, &args);
	if (!args.result || !sdm_object_is_a(args.result, type))
		return(NULL);
	return(args.result);
}

static inline struct sdm_object *sdm_thing_do_object_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, struct sdm_thing *target, struct sdm_object_type *type) {
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = "";

	sdm_thing_do_action(thing, action, &args);
	if (!args.result || !sdm_object_is_a(args.result, type))
		return(NULL);
	return(args.result);
}

#endif

