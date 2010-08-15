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
#include <sdm/actions/builtin/builtin.h>

int moo_load_basic_actions(MooBuiltinHash *);

int moo_basics_notify(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_announce(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_say(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_look(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_examine(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_go(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_direction(MooAction *action, MooThing *thing, MooArgs *args);

int moo_basics_inventory(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_get(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_drop(MooAction *action, MooThing *thing, MooArgs *args);

int moo_basics_room_do_enter(MooAction *action, MooThing *thing, MooArgs *args);
int moo_basics_room_do_leave(MooAction *action, MooThing *thing, MooArgs *args);

#endif

