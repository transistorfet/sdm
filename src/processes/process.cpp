/*
 * Object Name:	process.c
 * Description:	Process Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/processes/process.h>

MooObjectType moo_process_obj_type = {
	NULL,
	"process",
	(moo_type_create_t) moo_process_create
};

MooObject *moo_process_create(void)
{
	return(new MooProcess());
}


