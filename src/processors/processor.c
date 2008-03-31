/*
 * Object Name:	processor.c
 * Description:	Input Processor Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>

struct sdm_processor_type sdm_processor_obj_type = { {
	NULL,
	"processor",
	sizeof(struct sdm_processor),
	NULL,
	(sdm_object_init_t) sdm_processor_init,
	(sdm_object_release_t) sdm_processor_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL	},
	NULL,
	NULL,
	NULL
};

int sdm_processor_init(struct sdm_processor *processor, int nargs, va_list va)
{
	return(0);
}

void sdm_processor_release(struct sdm_processor *processor)
{

}


