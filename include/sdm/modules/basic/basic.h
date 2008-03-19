/*
 * Header Name:	basic.h
 * Description:	Basic Stuff Header
 */

#ifndef _SDM_MODULES_BASIC_H
#define _SDM_MODULES_BASIC_H

#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>

#define SDM_BASIC(ptr)		( (struct sdm_basic *) (ptr) )

struct sdm_basic {
	struct sdm_action action;
};

extern struct sdm_object_type sdm_basic_obj_type;

#define SDM_BASIC_ARGS(func)			SDM_ACTION_ARGS(func)

int init_basic(void);
int release_basic(void);

int sdm_basic_read_entry(struct sdm_basic *, const char *, struct sdm_data_file *);
int sdm_basic_write_data(struct sdm_basic *, struct sdm_data_file *);

int sdm_basic_action_tell_user(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);
int sdm_basic_action_announce(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);
int sdm_basic_action_say(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);
int sdm_basic_action_look(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);
int sdm_basic_action_examine(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);
int sdm_basic_action_move(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);

#endif

