/*
 * Object Name:	mobil.c
 * Description:	Base Self-Movable Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/mobile.h>

struct sdm_object_type sdm_mobile_obj_type = {
	sizeof(struct sdm_mobile),
	NULL,
	(sdm_object_init_t) sdm_mobile_init,
	(sdm_object_release_t) sdm_mobile_release
};

int sdm_mobile_init(struct sdm_mobile *mobile, va_list va)
{
	// TODO initialize mobile
	return(0);
}

void sdm_mobile_release(struct sdm_mobile *mobile)
{
	// TODO do all the other releasing
}


