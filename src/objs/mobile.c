/*
 * Object Name:	mobile.c
 * Description:	Base Self-Movable Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>
#include <sdm/objs/container.h>
#include <sdm/objs/mobile.h>

struct sdm_object_type sdm_mobile_obj_type = {
	&sdm_container_obj_type,
	sizeof(struct sdm_mobile),
	NULL,
	(sdm_object_init_t) sdm_mobile_init,
	(sdm_object_release_t) sdm_mobile_release
};

int sdm_mobile_init(struct sdm_mobile *mobile, va_list va)
{
	if (sdm_actionable_init(SDM_ACTIONABLE(mobile), va))
		return(-1);
	// TODO initialize mobile
	return(0);
}

void sdm_mobile_release(struct sdm_mobile *mobile)
{
	// TODO do all the other releasing
	sdm_actionable_release(SDM_ACTIONABLE(mobile));
}


