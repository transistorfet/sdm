/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/string.h>
#include <sdm/objs/container.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

struct sdm_action {
	sdm_action_t func;
	void *ptr;
	destroy_t destroy;
};

struct sdm_object_type sdm_thing_obj_type = {
	NULL,
	sizeof(struct sdm_thing),
	NULL,
	(sdm_object_init_t) sdm_thing_init,
	(sdm_object_release_t) sdm_thing_release
};

static void sdm_thing_destroy_action(struct sdm_action *);

int sdm_thing_init(struct sdm_thing *thing, va_list va)
{
	// TODO should we take "parent" as an argument?
	if (!(thing->properties = create_sdm_hash(SDM_BBF_CASE_INSENSITIVE, (destroy_t) destroy_sdm_object)))
		return(-1);
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(thing->actions = create_sdm_hash(SDM_BBF_CASE_INSENSITIVE, (destroy_t) sdm_thing_destroy_action)))
		return(-1);
	return(0);
}

void sdm_thing_release(struct sdm_thing *thing)
{
	if (thing->owner)
		sdm_container_remove(thing->owner, thing);
	if (thing->properties)
		destroy_sdm_hash(thing->properties);
	if (thing->actions)
		destroy_sdm_hash(thing->actions);
}

int sdm_thing_read_data(struct sdm_thing *thing, const char *type, struct sdm_data_file *data)
{
	struct sdm_object *obj;
	char buffer[STRING_SIZE];

	if (!strcmp(type, "string")) {
		if ((sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		    || !(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0)
			return(-1);
		sdm_thing_set_property(thing, buffer, obj);
	}
	else if (!strcmp(type, "action")) {
		// TODO how will you do this
/*
		if ((sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		    || !(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0)
			return(-1);
		sdm_thing_set_property(thing, buffer, obj);
*/
	}
	else if (!strcmp(type, "parent")) {
		// TODO find the parent
	}
	return(0);
}


int sdm_thing_set_property(struct sdm_thing *thing, const char *name, struct sdm_object *obj)
{
	if (sdm_hash_find(thing->properties, name))
		return(sdm_hash_replace(thing->properties, name, obj));
	return(sdm_hash_add(thing->properties, name, obj));
}

struct sdm_object *sdm_thing_get_property(struct sdm_thing *thing, const char *name, struct sdm_object_type *type)
{
	struct sdm_object *obj;

	if (!(obj = sdm_hash_find(thing->properties, name)))
		return(NULL);
	if (!type || (obj->type == type))
		return(obj);
	return(NULL);
}


int sdm_thing_set_action(struct sdm_thing *thing, const char *name, sdm_action_t func, void *ptr, destroy_t destroy)
{
	int res;
	struct sdm_action *action;

	if (!(action = (struct sdm_action *) memory_alloc(sizeof(struct sdm_action))))
		return(-1);
	action->func = func;
	action->ptr = ptr;
	action->destroy = destroy;
	if (sdm_hash_find(thing->actions, name))
		res = sdm_hash_replace(thing->actions, name, action);
	else
		res = sdm_hash_add(thing->actions, name, action);

	if (res >= 0)
		return(0);
	memory_free(action);
	return(-1);
}

int sdm_thing_do_action(struct sdm_thing *thing, struct sdm_user *user, const char *name, const char *args)
{
	struct sdm_action *action;
	struct sdm_thing *cur;

	for (cur = thing; cur; cur = cur->parent) {
		if ((action = sdm_hash_find(cur->actions, name))) {
			action->func(action->ptr, user, thing, args);
			return(0);
		}
	}
	return(-1);
}


/*** Local Functions ***/

static void sdm_thing_destroy_action(struct sdm_action *action)
{
	if (action->destroy)
		action->destroy(action->ptr);
	memory_free(action);
}


