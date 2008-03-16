/*
 * Object Name:	container.c
 * Description:	Actionable Container Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/string.h>

#include <sdm/objs/user.h>
#include <sdm/objs/world.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/container.h>

struct sdm_object_type sdm_container_obj_type = {
	&sdm_thing_obj_type,
	sizeof(struct sdm_container),
	NULL,
	(sdm_object_init_t) sdm_container_init,
	(sdm_object_release_t) sdm_container_release,
	(sdm_object_read_entry_t) sdm_container_read_entry,
	(sdm_object_write_data_t) sdm_container_write_data
};

int sdm_container_init(struct sdm_container *container, va_list va)
{
	if (sdm_thing_init(SDM_THING(container), va))
		return(-1);
	return(0);
}

void sdm_container_release(struct sdm_container *container)
{
	struct sdm_thing *cur, *tmp;

	for (cur = container->objects; cur; cur = tmp) {
		tmp = cur->next;
		destroy_sdm_object(SDM_OBJECT(cur));
	}
	sdm_thing_release(SDM_THING(container));
}

int sdm_container_read_entry(struct sdm_container *container, const char *type, struct sdm_data_file *data)
{
	struct sdm_thing *obj;

	if (!strcmp(type, "thing")) {
		if (!(obj = (struct sdm_thing *) create_sdm_object(&sdm_thing_obj_type, SDM_THING_ARGS(SDM_NO_ID, 0))))
			return(-1);
		sdm_data_read_children(data);
		sdm_object_read_data(SDM_OBJECT(obj), data);
		sdm_data_read_parent(data);
		sdm_container_add(container, obj);
	}
	else if (!strcmp(type, "container")) {
		if (!(obj = (struct sdm_thing *) create_sdm_object(&sdm_container_obj_type, SDM_CONTAINER_ARGS(SDM_NO_ID, 0))))
			return(-1);
		sdm_data_read_children(data);
		sdm_object_read_data(SDM_OBJECT(obj), data);
		sdm_data_read_parent(data);
		sdm_container_add(container, obj);
	}
	else
		return(SDM_NOT_HANDLED);
	return(SDM_HANDLED);
}

int sdm_container_write_data(struct sdm_container *container, struct sdm_data_file *data)
{
	struct sdm_thing *cur;

	for (cur = container->objects; cur; cur = cur->next) {
		if (sdm_object_is_a(SDM_OBJECT(cur), &sdm_user_obj_type))
			continue;
		else if (sdm_object_is_a(SDM_OBJECT(cur), &sdm_world_obj_type)) {
			sdm_world_write(SDM_WORLD(cur), data);
			/** If we don't continue here, the world will be written to this file which we don't
			    because the world object is a reference only and is written to it's own file */
			continue;
		}
		else if (sdm_object_is_a(SDM_OBJECT(cur), &sdm_container_obj_type))
			sdm_data_write_begin_entry(data, "container");
		else
			sdm_data_write_begin_entry(data, "thing");
		sdm_object_write_data(SDM_OBJECT(cur), data);
		sdm_data_write_end_entry(data);
	}
	return(0);
}


int sdm_container_add(struct sdm_container *container, struct sdm_thing *obj)
{
	// TODO should you notify the objcet somehow that it's being added? (or are these funcs lower level)
	if (obj->owner == container)
		return(0);
	if (obj->owner)
		sdm_container_remove(obj->owner, obj);
	obj->owner = container;
	obj->next = container->objects;
	container->objects = obj;
	return(0);
}

int sdm_container_remove(struct sdm_container *container, struct sdm_thing *obj)
{
	struct sdm_thing *cur, *prev;

	// TODO should you notify the object somehow that it is being remove?
	for (prev = NULL, cur = container->objects; cur; prev = cur, cur = cur->next) {
		if (cur == obj) {
			if (prev)
				prev->next = cur->next;
			else
				container->objects = cur->next;
			cur->owner = NULL;
			return(0);
		}
	}
	return(-1);
}


struct sdm_thing *sdm_container_find(struct sdm_container *container, const char *name)
{
	struct sdm_thing *cur;
	struct sdm_object *obj;

	for (cur = container->objects; cur; cur = cur->next) {
		if (!(obj = sdm_thing_get_property(cur, "name", &sdm_string_obj_type)))
			continue;
		// TODO check alternative names stored in properties
		// TODO keep looking after a match is found in order to find a possibly better match
		if (!strncasecmp(SDM_STRING(obj)->str, name, strlen(name)))
			return(cur);
	}
	return(NULL);
}


