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
	"string",
	sizeof(struct sdm_string),
	NULL,
	(sdm_object_init_t) sdm_string_init,
	(sdm_object_release_t) sdm_string_release,
	(sdm_object_read_entry_t) sdm_string_read_entry,
	(sdm_object_write_data_t) sdm_string_write_data
};

int init_sdm_string_type(void)
{
	if (sdm_object_register_type(&sdm_string_obj_type) < 0)
		return(-1);
	return(0);
}

void release_sdm_string_type(void)
{
	sdm_object_deregister_type(&sdm_string_obj_type);
}


int sdm_string_init(struct sdm_string *string, int nargs, va_list va)
{
	char *fmt;
	char buffer[LARGE_STRING_SIZE];

	if (nargs <= 0)
		return(0);
	fmt = va_arg(va, char *);
	vsnprintf(buffer, LARGE_STRING_SIZE, fmt, va);
	string->len = strlen(buffer);
	if (!(string->str = memory_alloc(string->len + 1)))
		return(-1);
	strcpy(string->str, buffer);
	return(0);
}

void sdm_string_release(struct sdm_string *string)
{
	memory_free(string->str);
}

int sdm_string_read_entry(struct sdm_string *string, const char *type, struct sdm_data_file *data)
{
	char buffer[STRING_SIZE];

	sdm_data_read_string(data, buffer, STRING_SIZE);
	if (string->str)
		memory_free(string->str);
	string->len = strlen(buffer);
	if (!(string->str = memory_alloc(string->len + 1)))
		return(-1);
	strcpy(string->str, buffer);
	return(SDM_HANDLED_ALL);
}

int sdm_string_write_data(struct sdm_string *string, struct sdm_data_file *data)
{
	sdm_data_write_string(data, string->str);
	return(0);
}


