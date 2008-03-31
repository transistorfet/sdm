/*
 * Object Name:	list.c
 * Description:	List Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/list.h>

struct sdm_list_entry {
	struct sdm_object *obj;
	struct sdm_list_entry *next;
};

struct sdm_object_type sdm_list_obj_type = {
	NULL,
	"list",
	sizeof(struct sdm_list),
	NULL,
	(sdm_object_init_t) sdm_list_init,
	(sdm_object_release_t) sdm_list_release,
	(sdm_object_read_entry_t) sdm_list_read_entry,
	(sdm_object_write_data_t) sdm_list_write_data
};

int init_sdm_list_type(void)
{
	if (sdm_object_register_type(&sdm_list_obj_type) < 0)
		return(-1);
	return(0);
}

void release_sdm_list_type(void)
{
	sdm_object_deregister_type(&sdm_list_obj_type);
}


int sdm_list_init(struct sdm_list *list, int nargs, va_list va)
{
	return(0);
}

void sdm_list_release(struct sdm_list *list)
{
	struct sdm_list_entry *cur, *next;

	for (cur = list->head, next = cur->next; cur; cur = next, next = cur->next) {
		destroy_sdm_object(cur->obj);
		memory_free(cur);
	}
}

int sdm_list_read_entry(struct sdm_list *list, const char *type, struct sdm_data_file *data)
{

	return(0);
}

int sdm_list_write_data(struct sdm_list *list, struct sdm_data_file *data)
{

	return(0);
}

int sdm_list_add(struct sdm_list *list, struct sdm_object *obj)
{
	struct sdm_list_entry *entry;

	if (!(entry = (struct sdm_list_entry *) memory_alloc(sizeof(struct sdm_list_entry))))
		return(-1);
	entry->obj = obj;
	entry->next = NULL;
	if (list->tail) {
		list->tail->next = entry;
		list->tail = entry;
	}
	else {
		list->head = entry;
		list->tail = entry;
	}
	return(0);
}

void sdm_list_remove(struct sdm_list *list, struct sdm_object *obj)
{
	struct sdm_list_entry *cur, *prev;

	for (prev = NULL, cur = list->head; cur; prev = cur, cur = cur->next) {
		if (cur->obj == obj) {
			if (list->traverse_next == cur)
				list->traverse_next = cur->next;
			if (prev)
				prev->next = cur->next;
			else
				list->head = cur->next;
			if (list->tail == cur)
				list->tail = prev;
			memory_free(cur);
			return;
		}
	}
}

void sdm_list_delete(struct sdm_list *list, struct sdm_object *obj)
{
	sdm_list_remove(list, obj);
	destroy_sdm_object(obj);
}


int sdm_list_traverse_reset(struct sdm_list *list)
{
	list->traverse_next = list->head;
	return(0);
}

struct sdm_object *sdm_list_traverse_next(struct sdm_list *list)
{
	struct sdm_object *obj;

	if (!list->traverse_next)
		return(NULL);
	obj = list->traverse_next->obj;
	list->traverse_next = list->traverse_next->next;
	return(obj);
}


