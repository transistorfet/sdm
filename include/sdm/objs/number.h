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

int sdm_number_init(struct sdm_number *, va_list);
void sdm_number_release(struct sdm_number *);

#endif

