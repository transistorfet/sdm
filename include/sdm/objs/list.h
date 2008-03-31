/*
 * Object Name:	list.h
 * Description:	List Object
 */

#ifndef _SDM_OBJS_LIST_H
#define _SDM_OBJS_LIST_H

#include <stdarg.h>

#include <sdm/objs/object.h>

#define SDM_LIST(ptr)		( (struct sdm_list *) (ptr) )

struct sdm_list_entry;

struct sdm_list {
	struct sdm_object object;
	struct sdm_list_entry *head;
	struct sdm_list_entry *tail;
	struct sdm_list_entry *traverse_next;
};

extern struct sdm_object_type sdm_list_obj_type;

#define create_sdm_list(str)	\
	( SDM_LIST(create_sdm_object(&sdm_list_obj_type, 0)) )

int init_sdm_list_type(void);
void release_sdm_list_type(void);

int sdm_list_init(struct sdm_list *, int, va_list);
void sdm_list_release(struct sdm_list *);
int sdm_list_read_entry(struct sdm_list *, const char *, struct sdm_data_file *);
int sdm_list_write_data(struct sdm_list *, struct sdm_data_file *);

int sdm_list_add(struct sdm_list *, struct sdm_object *);
void sdm_list_remove(struct sdm_list *, struct sdm_object *);
void sdm_list_delete(struct sdm_list *, struct sdm_object *);

int sdm_list_traverse_reset(struct sdm_list *);
struct sdm_object *sdm_list_traverse_next(struct sdm_list *);

#endif

