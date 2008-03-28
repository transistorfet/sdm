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
	"number",
	sizeof(struct sdm_number),
	NULL,
	(sdm_object_init_t) sdm_number_init,
	(sdm_object_release_t) sdm_number_release,
	(sdm_object_read_entry_t) sdm_number_read_entry,
	(sdm_object_write_data_t) sdm_number_write_data
};

int init_sdm_number_type(void)
{
	if (sdm_object_register_type(&sdm_number_obj_type) < 0)
		return(-1);
	return(0);
}

void release_sdm_number_type(void)
{
	sdm_object_deregister_type(&sdm_number_obj_type);
}


int sdm_number_init(struct sdm_number *number, int nargs, va_list va)
{
	if (nargs > 0)
		number->num = va_arg(va, sdm_number_t);
	return(0);
}

void sdm_number_release(struct sdm_number *number)
{
	/** Nothing to free */
}

int sdm_number_read_entry(struct sdm_number *number, const char *type, struct sdm_data_file *data)
{
	number->num = sdm_data_read_float(data);
	return(SDM_HANDLED_ALL);
}

int sdm_number_write_data(struct sdm_number *number, struct sdm_data_file *data)
{
	sdm_data_write_float(data, number->num);
	return(0);
}

