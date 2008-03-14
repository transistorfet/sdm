/*
 * Object Name:	number.c
 * Description:	Number Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/number.h>

struct sdm_object_type sdm_number_obj_type = {
	NULL,
	sizeof(struct sdm_number),
	NULL,
	(sdm_object_init_t) sdm_number_init,
	(sdm_object_release_t) sdm_number_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL
};

int sdm_number_init(struct sdm_number *number, va_list va)
{
	number->num = va_arg(va, sdm_number_t);
	return(0);
}

void sdm_number_release(struct sdm_number *number)
{
	/** Nothing to free */
}


