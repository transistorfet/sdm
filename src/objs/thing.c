/*
 * Object Name:	actionable.c
 * Description:	Base Actionable Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/string.h>
#include <sdm/objs/container.h>

#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>

struct sdm_action {
	sdm_action_t func;
	void *ptr;
	destroy_t destroy;
};

struct sdm_object_type sdm_actionable_obj_type = {
	NULL,
	sizeof(struct sdm_actionable),
	NULL,
	(sdm_object_init_t) sdm_actionable_init,
	(sdm_object_release_t) sdm_actionable_release
};

static void sdm_actionable_destroy_action(struct sdm_action *);

int sdm_actionable_init(struct sdm_actionable *actionable, va_list va)
{
	// TODO should we take "parent" as an argument?
	if (!(actionable->properties = create_sdm_hash(SDM_BBF_CASE_INSENSITIVE, (destroy_t) destroy_sdm_object)))
		return(-1);
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(actionable->actions = create_sdm_hash(SDM_BBF_CASE_INSENSITIVE, (destroy_t) sdm_actionable_destroy_action)))
		return(-1);
	return(0);
}

void sdm_actionable_release(struct sdm_actionable *actionable)
{
	if (actionable->owner)
		sdm_container_remove(actionable->owner, actionable);
	if (actionable->properties)
		destroy_sdm_hash(actionable->properties);
	if (actionable->actions)
		destroy_sdm_hash(actionable->actions);
}

int sdm_actionable_read_data(struct sdm_actionable *actionable, const char *type, struct sdm_data_file *data)
{
	struct sdm_object *obj;
	char buffer[STRING_SIZE];

	if (!strcmp(type, "string")) {
		if ((sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		    || !(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0)
			return(-1);
		sdm_actionable_set_property(actionable, buffer, obj);
	}
	else if (!strcmp(type, "action")) {
		// TODO how will you do this
/*
		if ((sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		    || !(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0)
			return(-1);
		sdm_actionable_set_property(actionable, buffer, obj);
*/
	}
	else if (!strcmp(type, "parent")) {
		// TODO find the parent
	}
	return(0);
}


int sdm_actionable_set_property(struct sdm_actionable *actionable, const char *name, struct sdm_object *obj)
{
	if (sdm_hash_find(actionable->properties, name))
		return(sdm_hash_replace(actionable->properties, name, obj));
	return(sdm_hash_add(actionable->properties, name, obj));
}

struct sdm_object *sdm_actionable_get_property(struct sdm_actionable *actionable, const char *name, struct sdm_object_type *type)
{
	struct sdm_object *obj;

	if (!(obj = sdm_hash_find(actionable->properties, name)))
		return(NULL);
	if (!type || (obj->type == type))
		return(obj);
	return(NULL);
}


int sdm_actionable_set_action(struct sdm_actionable *actionable, const char *name, sdm_action_t func, void *ptr, destroy_t destroy)
{
	int res;
	struct sdm_action *action;

	if (!(action = (struct sdm_action *) memory_alloc(sizeof(struct sdm_action))))
		return(-1);
	action->func = func;
	action->ptr = ptr;
	action->destroy = destroy;
	if (sdm_hash_find(actionable->actions, name))
		res = sdm_hash_replace(actionable->actions, name, action);
	else
		res = sdm_hash_add(actionable->actions, name, action);

	if (res >= 0)
		return(0);
	memory_free(action);
	return(-1);
}

int sdm_actionable_do_action(struct sdm_actionable *actionable, struct sdm_user *user, const char *name, const char *args)
{
	struct sdm_action *action;
	struct sdm_actionable *cur;

	for (cur = actionable; cur; cur = cur->parent) {
		if ((action = sdm_hash_find(cur->actions, name))) {
			action->func(action->ptr, user, actionable, args);
			return(0);
		}
	}
	return(-1);
}


/*** Local Functions ***/

static void sdm_actionable_destroy_action(struct sdm_action *action)
{
	if (action->destroy)
		action->destroy(action->ptr);
	memory_free(action);
}


