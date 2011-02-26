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
#include <sdm/actions/action.h>
#include <sdm/interfaces/interface.h>

#define TASK_LIST_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE

MooObjectType moo_task_obj_type = {
	NULL,
	"task",
	typeid(MooTask).name(),
	(moo_type_create_t) NULL
};

static moo_id_t g_current_owner = -1;
static MooTask *g_current_task = NULL;
static MooArray<MooTask *> *g_task_list = NULL;

int init_task(void)
{
	if (g_task_list)
		return(1);
	g_task_list = new MooArray<MooTask *>(MOO_ARRAY_DEFAULT_SIZE, -1, TASK_LIST_BITS);
	return(0);
}

void release_task(void)
{
	delete g_task_list;
}

MooTask::MooTask()
{
	m_tid = g_task_list->add(this);
	if (g_current_task)
		m_parent_tid = g_current_task->m_tid;
	else
		m_parent_tid = -1;
}

MooTask::~MooTask()
{
	if (this == g_current_task) {
		g_current_task = NULL;
		g_current_owner = -1;
	}
	g_task_list->set(m_tid, NULL);
}

int MooTask::bestow(MooInterface *inter)
{
	delete inter;
	return(-1);
}

int MooTask::elevated_do_action(MooAction *action, MooArgs *args)
{
	int res;
	MooTask *prev_task;
	moo_id_t prev_owner;

	if (!action)
		return(-1);
	/// We save the current task and when we restore the owner, we make sure the same task is running, so that we don't
	/// accidentally change the owner for a different task, giving it erronous permissions
	prev_task = MooTask::current_task();
	prev_owner = MooTask::current_owner();
	MooTask::current_owner(action->owner());
	try {
		res = action->do_action(args);
	}
	catch (int e) {
		MooTask::current_owner(prev_task, prev_owner);
		throw e;
	}
	catch (MooException e) {
		MooTask::current_owner(prev_task, prev_owner);
		throw e;
	}
	catch (...) {
		MooTask::current_owner(prev_task, prev_owner);
		throw MooException("Error in elevated do_action");
	}
	MooTask::current_owner(prev_task, prev_owner);
	return(res);
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
	return(g_current_owner);
}

moo_id_t MooTask::current_user()
{
	if (!g_current_task)
		return(-1);
	return(g_current_task->owner());
}

moo_id_t MooTask::current_owner(moo_id_t id)
{
	g_current_owner = id;
	return(g_current_owner);
}

moo_id_t MooTask::current_owner(MooTask *task, moo_id_t id)
{
	if (g_current_task != task)
		return(g_current_owner);
	g_current_owner = id;
	return(g_current_owner);
}

int MooTask::switch_task(MooTask *task)
{
	g_current_task = task;
	if (task)
		g_current_owner = task->owner();
	return(0);
}

