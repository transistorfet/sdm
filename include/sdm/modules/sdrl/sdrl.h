/*
 * Header Name:	sdrl.h
 * Description:	SDRL Scripting Module Header
 */

#ifndef _SDM_MODULES_SDRL_SDRL_H
#define _SDM_MODULES_SDRL_SDRL_H

#include <sdrl/sdrl.h>

#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/modules/sdrl/funcs.h>
#include <sdm/modules/sdrl/object.h>

#define SDM_SDRL(ptr)		( (struct sdm_sdrl *) (ptr) )

struct sdm_sdrl {
	struct sdm_action action;
	struct sdrl_expr *expr;
};

extern struct sdm_object_type sdm_sdrl_obj_type;

int init_sdrl(void);
void release_sdrl(void);

int sdm_sdrl_read_entry(struct sdm_sdrl *, const char *, struct sdm_data_file *);
int sdm_sdrl_write_data(struct sdm_sdrl *, struct sdm_data_file *);

int sdm_sdrl_action(struct sdm_sdrl *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);

#endif

