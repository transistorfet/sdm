/*
 * Object Name:	object.h
 * Description:	Base Object
 */

#ifndef _SDM_OBJS_OBJECT_H
#define _SDM_OBJS_OBJECT_H

#include <stdarg.h>
#include <sdm/data.h>

#define SDM_NOT_HANDLED		0
#define SDM_HANDLED		1
#define SDM_HANDLED_ALL		2

#define SDM_OBF_RELEASING	0x1000

#define SDM_OBJECT(ptr)		( (struct sdm_object *) (ptr) )
#define SDM_OBJECT_TYPE(ptr)	( (struct sdm_object_type *) (ptr) )

struct sdm_object;
struct sdm_object_type;

/** Initialize the preallocade object using the given variable args.  The object will be zero'd before
    the initialization function is called.  If a negative number is returned, the object will destroyed
    and the release function for that object will be called. */
typedef int (*sdm_object_init_t)(struct sdm_object *, va_list);
/** Release the resources freed by the object */
typedef void (*sdm_object_release_t)(struct sdm_object *);
/** Read an entry from the given open data handle and load the information into the object.  The name of
    the current entry is given to avoid another call to sdm_data_read_name().  If an error occurs, a
    negative number is returned.  If the entry is read successfully, a positive number is returned.  If
    the entry type is not loadable/recognized by the object, 0 should be returned and the caller shall
    call the read function of the parent in order to read the entry.  This function should not recursively
    call the corresponding function of it's parent object */
typedef int (*sdm_object_read_entry_t)(struct sdm_object *, const char *, struct sdm_data_file *);
/** Write all data for the object to the given open data handle.  Only data for the immediate object will
    be written and not data for the object's parent.  The caller shall call the write function for the
    object's parent before calling this function.  If an error occurs, a negative number is returned. */
typedef int (*sdm_object_write_data_t)(struct sdm_object *, struct sdm_data_file *);

struct sdm_object {
	struct sdm_object_type *type;
	int bitflags;

};

struct sdm_object_type {
	struct sdm_object_type *parent;
	char *name;
	int size;
	void *ptr;
	sdm_object_init_t init;
	sdm_object_release_t release;
	sdm_object_read_entry_t read_entry;
	sdm_object_write_data_t write_data;
};

int init_object(void);
void release_object(void);

int sdm_object_register_type(struct sdm_object_type *);
int sdm_object_deregister_type(struct sdm_object_type *);
struct sdm_object_type *sdm_object_find_type(const char *, struct sdm_object_type *);

struct sdm_object *create_sdm_object(struct sdm_object_type *, ...);
void destroy_sdm_object(struct sdm_object *);

int sdm_object_read_file(struct sdm_object *, const char *, const char *);
int sdm_object_write_file(struct sdm_object *, const char *, const char *);
int sdm_object_read_data(struct sdm_object *, struct sdm_data_file *);
int sdm_object_write_data(struct sdm_object *, struct sdm_data_file *);

static inline int sdm_object_is_a(struct sdm_object *obj, struct sdm_object_type *type) {
	struct sdm_object_type *cur;

	for (cur = obj->type; cur; cur = cur->parent) {
		if (cur == type)
			return(1);
	}
	return(0);
}

#endif

