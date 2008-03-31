/*
 * Object Name:	mobile.c
 * Description:	Base Self-Movable Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/mobile.h>

struct sdm_object_type sdm_mobile_obj_type = {
	&sdm_thing_obj_type,
	"mobile",
	sizeof(struct sdm_mobile),
	NULL,
	(sdm_object_init_t) sdm_mobile_init,
	(sdm_object_release_t) sdm_mobile_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL
};

int sdm_mobile_init(struct sdm_mobile *mobile, int nargs, va_list va)
{
	if (sdm_thing_init(SDM_THING(mobile), nargs, va))
		return(-1);
	// TODO initialize mobile
	return(0);
}

void sdm_mobile_release(struct sdm_mobile *mobile)
{
	// TODO do all the other releasing
	sdm_thing_release(SDM_THING(mobile));
}


