/*
 * Object Name:	action.c
 * Description:	Action Base Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/action.h>

struct sdm_object_type sdm_action_obj_type = {
	NULL,
	"action",
	sizeof(struct sdm_action),
	NULL,
	(sdm_object_init_t) sdm_action_init,
	(sdm_object_release_t) sdm_action_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL
};

int sdm_action_init(struct sdm_action *action, va_list va)
{
	action->func = va_arg(va, sdm_action_t);
	return(0);
}

void sdm_action_release(struct sdm_action *action)
{

}


