/*
 * Object Name:	container.c
 * Description:	Actionable Container Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>
#include <sdm/objs/container.h>

struct sdm_object_type sdm_container_obj_type = {
	sizeof(struct sdm_container),
	NULL,
	(sdm_object_init_t) sdm_container_init,
	(sdm_object_release_t) sdm_container_release
};

int sdm_container_init(struct sdm_container *container, va_list va)
{
	if (sdm_actionable_init(SDM_ACTIONABLE(container), va))
		return(-1);
	return(0);
}

void sdm_container_release(struct sdm_container *container)
{
	struct sdm_actionable *cur, *tmp;

	for (cur = container->objects; cur; cur = tmp) {
		tmp = cur->next;
		destroy_sdm_object(SDM_OBJECT(cur));
	}
	sdm_actionable_release(SDM_ACTIONABLE(container));
}

int sdm_container_add(struct sdm_container *container, struct sdm_actionable *obj)
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

int sdm_container_remove(struct sdm_container *container, struct sdm_actionable *obj)
{
	struct sdm_actionable *cur, *prev;

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


struct sdm_actionable *sdm_container_find(struct sdm_container *container, const char *name)
{
	struct sdm_actionable *cur;

	for (cur = container->objects; cur; cur = cur->next) {
// TODO where is the name stored? should all actionable objects have a name? or should it be from properties?
//		if (!strncasecmp(cur->name, name, strlen(name)))
	}
	return(NULL);
}


