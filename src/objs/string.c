/*
 * Object Name:	string.c
 * Description:	String Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/string.h>

struct sdm_object_type sdm_string_obj_type = {
	NULL,
	sizeof(struct sdm_string),
	NULL,
	(sdm_object_init_t) sdm_string_init,
	(sdm_object_release_t) sdm_string_release
};

int sdm_string_init(struct sdm_string *string, va_list va)
{
	char *fmt;
	char buffer[LARGE_STRING_SIZE];

	fmt = va_arg(va, char *);
	vsnprintf(buffer, LARGE_STRING_SIZE, fmt, va);
	if (!(string->str = memory_alloc(strlen(buffer) + 1)))
		return(-1);
	strcpy(string->str, buffer);
	return(0);
}

void sdm_string_release(struct sdm_string *string)
{
	memory_free(string->str);
}


