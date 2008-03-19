/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_OBJS_ACTION_H
#define _SDM_OBJS_ACTION_H

#include <stdarg.h>

#include <sdm/objs/object.h>

#define SDM_ACTION(ptr)		( (struct sdm_action *) (ptr) )

struct sdm_thing;
struct sdm_action;

typedef int (*sdm_action_t)(struct sdm_action *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);

struct sdm_action {
	struct sdm_object object;
	sdm_action_t func;
};

extern struct sdm_object_type sdm_action_obj_type;

#define SDM_ACTION_ARGS(action)			(action)

int sdm_action_init(struct sdm_action *, va_list);
void sdm_action_release(struct sdm_action *);

#endif

