/*
 * Header Name:	basics.h
 * Description:	Basic Actions Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BASICS_H
#define _SDM_ACTIONS_BUILTIN_BASICS_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/actions/action.h>

int sdm_builtin_load_basics(struct sdm_hash *);

int sdm_builtin_action_notify(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_announce(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_say(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_look(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_examine(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_go(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_direction(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

int sdm_builtin_action_inventory(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_get(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);
int sdm_builtin_action_drop(struct sdm_action *, struct sdm_thing *, struct sdm_action_args *);

#endif

