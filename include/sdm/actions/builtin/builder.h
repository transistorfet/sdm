/*
 * Header Name:	basics.h
 * Description:	Basic Actions Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BUILDER_H
#define _SDM_ACTIONS_BUILTIN_BUILDER_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/actions/action.h>

int sdm_builtin_load_builder(struct sdm_hash *);

int sdm_builtin_action_create(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_create_room(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_add_exit(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_set(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

#endif

