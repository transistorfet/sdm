/*
 * Object Name:	actionable.h
 * Description:	Base Actionable Object
 */

#ifndef _SDM_OBJS_ACTIONABLE_H
#define _SDM_OBJS_ACTIONABLE_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/objs/object.h>

#define SDM_ACTIONABLE(ptr)	( (struct sdm_actionable *) (ptr) )

struct sdm_user;
struct sdm_container;
struct sdm_actionable;

typedef int (*sdm_action_t)(void *, struct sdm_user *, struct sdm_actionable *, const char *);

struct sdm_actionable {
	struct sdm_object object;
	struct sdm_actionable *parent;
	struct sdm_container *owner;
	struct sdm_actionable *next;
	struct sdm_hash *properties;
	struct sdm_hash *actions;

};

extern struct sdm_object_type sdm_actionable_obj_type;

int sdm_actionable_init(struct sdm_actionable *, va_list);
void sdm_actionable_release(struct sdm_actionable *);

int sdm_actionable_set_action(struct sdm_actionable *, const char *, sdm_action_t, void *, destroy_t);
int sdm_actionable_do_action(struct sdm_actionable *, struct sdm_user *, const char *, const char *);

#endif

