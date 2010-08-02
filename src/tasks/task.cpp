/*
 * Object Name:	task.c
 * Description:	Task Object
 */

#include <stdarg.h>

#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/tasks/task.h>

MooObjectType moo_task_obj_type = {
	NULL,
	"task",
	typeid(MooTask).name(),
	(moo_type_create_t) NULL
};

static MooArray<MooTask *> *task_list = NULL;

int init_task(void)
{
	if (task_list)
		return(1);
	task_list = new MooArray<MooTask *>();
	return(0);
}

void release_task(void)
{
	delete task_list;
}

MooTask::MooTask()
{

}

MooTask::~MooTask()
{

}


