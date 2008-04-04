/*
 * Header Name:	basic.h
 * Description:	Basic Stuff Header
 */

#ifndef _SDM_ACTIONS_BASIC_BASIC_H
#define _SDM_ACTIONS_BASIC_BASIC_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

#define SDM_BASIC(ptr)		( (struct sdm_basic *) (ptr) )

struct sdm_basic {
	struct sdm_action action;
	struct sdm_hash_entry *entry;
};

extern struct sdm_object_type sdm_basic_obj_type;

#define SDM_BASIC_ARGS(func)			SDM_ACTION_ARGS(func)

int init_basic(void);
void release_basic(void);

int sdm_basic_read_entry(struct sdm_basic *, const char *, struct sdm_data_file *);
int sdm_basic_write_data(struct sdm_basic *, struct sdm_data_file *);

int sdm_basic_action_notify(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_announce(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_say(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_look(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_examine(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_move(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

int sdm_basic_action_inventory(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_get(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_basic_action_drop(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

int sdm_basic_load_builder(struct sdm_hash *);

#endif

