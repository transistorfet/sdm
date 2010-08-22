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

static MooTask *g_current_task = NULL;
static MooArray<MooTask *> *g_task_list = NULL;

int init_task(void)
{
	if (g_task_list)
		return(1);
	g_task_list = new MooArray<MooTask *>();
	return(0);
}

void release_task(void)
{
	delete g_task_list;
}

MooTask::MooTask()
{
	m_tid = g_task_list->add(this);
	if (g_current_task) {
		m_owner = g_current_task->m_owner;
		m_parent_tid = g_current_task->m_tid;
	}
	else {
		m_owner = 0;
		m_parent_tid = -1;
	}
}

MooTask::~MooTask()
{
	if (this == g_current_task)
		g_current_task = NULL;
	g_task_list->set(m_tid, NULL);
}

int MooTask::bestow(MooInterface *inter)
{
	delete inter;
	return(-1);
}

moo_id_t MooTask::owner(moo_id_t id)
{
	if (id > 0) {
		// TODO do check for if current task has permissions to set the owner to this
		m_owner = id;
	}
	return(m_owner);
}

int MooTask::switch_handle(MooInterface *inter, int ready)
{
	MooTask::switch_task(this);
	return(this->handle(inter, ready));
}

MooTask *MooTask::current_task()
{
	return(g_current_task);
}

moo_id_t MooTask::current_owner()
{
	if (!g_current_task)
		return(-1);
	return(g_current_task->m_owner);
}

int MooTask::switch_task(MooTask *task)
{
	g_current_task = task;
	return(0);
}

