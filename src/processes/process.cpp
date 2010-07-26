/*
 * Object Name:	process.c
 * Description:	Process Object
 */

#include <stdarg.h>

#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/processes/process.h>

MooObjectType moo_process_obj_type = {
	NULL,
	"process",
	(moo_type_create_t) moo_process_create
};

static MooArray<MooProcess *> process_list = NULL;

int init_process(void)
{
	if (process_list)
		return(1);
	process_list = new MooArray<MooProcess *>();
}

void release_process(void)
{
	delete process_list;
}

MooObject *moo_process_create(void)
{
	return(new MooProcess());
}

MooProcess::MooProcess()
{

}

MooProcess::~MooProcess()
{

}


