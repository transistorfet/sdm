/*
 * Object Name:	number.h
 * Description:	Number Object
 */

#ifndef _SDM_OBJS_NUMBER_H
#define _SDM_OBJS_NUMBER_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef double sdm_number_t;

#define SDM_NUMBER(ptr)		( (struct sdm_number *) (ptr) )

struct sdm_number {
	struct sdm_object object;
	sdm_number_t num;
};

extern struct sdm_object_type sdm_number_obj_type;

#define create_sdm_number(num)	\
	( SDM_NUMBER(create_sdm_object(&sdm_number_obj_type, 1, (num))) )

int init_sdm_number_type(void);
void release_sdm_number_type(void);

int sdm_number_init(struct sdm_number *, int, va_list);
void sdm_number_release(struct sdm_number *);
int sdm_number_read_entry(struct sdm_number *, const char *, struct sdm_data_file *);
int sdm_number_write_data(struct sdm_number *, struct sdm_data_file *);

#endif

