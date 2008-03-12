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

int sdm_string_init(struct sdm_string *, va_list);
void sdm_string_release(struct sdm_string *);

#endif

