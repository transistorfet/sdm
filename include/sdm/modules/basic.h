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

int init_basic(void);
int release_basic(void);

int sdm_basic_read_action(struct sdm_thing *, struct sdm_data_file *);

int sdm_basic_action_look(void *, struct sdm_user *, struct sdm_thing *, const char *);

#endif

