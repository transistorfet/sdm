/*
 * Header Name:	builtin.h
 * Description:	Builtin Stuff Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BUILTIN_H
#define _SDM_ACTIONS_BUILTIN_BUILTIN_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define SDM_BUILTIN(ptr)		( (struct sdm_builtin *) (ptr) )

struct sdm_builtin {
	struct sdm_action action;
	struct sdm_hash_entry *entry;
};

extern struct sdm_object_type sdm_builtin_obj_type;

#define SDM_BUILTIN_ARGS(func)			SDM_ACTION_ARGS(func)

int init_builtin(void);
void release_builtin(void);

int sdm_builtin_read_entry(struct sdm_builtin *, const char *, struct sdm_data_file *);
int sdm_builtin_write_data(struct sdm_builtin *, struct sdm_data_file *);

#endif

