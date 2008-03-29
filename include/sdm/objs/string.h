/*
 * Object Name:	string.h
 * Description:	String Object
 */

#ifndef _SDM_OBJS_STRING_H
#define _SDM_OBJS_STRING_H

#include <stdarg.h>

#include <sdm/objs/object.h>

#define SDM_STRING(ptr)		( (struct sdm_string *) (ptr) )

struct sdm_string {
	struct sdm_object object;
	char *str;
	int len;
};

extern struct sdm_object_type sdm_string_obj_type;

#define create_sdm_string(str)	\
	( SDM_STRING(create_sdm_object(&sdm_string_obj_type, 1, (str))) )

int init_sdm_string_type(void);
void release_sdm_string_type(void);

int sdm_string_init(struct sdm_string *, int, va_list);
void sdm_string_release(struct sdm_string *);
int sdm_string_read_entry(struct sdm_string *, const char *, struct sdm_data_file *);
int sdm_string_write_data(struct sdm_string *, struct sdm_data_file *);

int sdm_string_set(struct sdm_string *, const char *);

#endif

