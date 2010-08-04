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
#include <sdm/interfaces/interface.h>

MooObjectType moo_task_obj_type = {
	NULL,
	"task",
	typeid(MooTask).name(),
	(moo_type_create_t) NULL
};

static MooTask *current_task = NULL;
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
	m_tid = task_list->add(this);
	if (current_task)
		m_parent_tid = current_task->m_tid;
	else
		m_parent_tid = -1;
}

MooTask::~MooTask()
{
	task_list->set(m_tid, NULL);
}

int MooTask::bestow(MooInterface *inter)
{
	delete inter;
	return(-1);
}

