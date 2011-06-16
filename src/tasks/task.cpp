/*
 * Object Name:	task.c
 * Description:	Task Object
 */

#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>
#include <sdm/code/code.h>

#include <sdm/tasks/init.h>

#define TASK_LIST_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE

MooObjectType moo_task_obj_type = {
	NULL,
	"task",
	typeid(MooTask).name(),
	(moo_type_make_t) NULL
};

static moo_id_t g_current_owner = -1;
static MooTask *g_current_task = NULL;
static MooArray<MooTask *> *g_task_list = NULL;
static MooInit *g_init_task = NULL;

int init_task(void)
{
	if (g_task_list)
		return(1);
	g_task_list = new MooArray<MooTask *>(MOO_ARRAY_DEFAULT_SIZE, -1, TASK_LIST_BITS);
	g_init_task = new MooInit();
	g_init_task->owner(0);
	g_current_task = g_init_task;
	g_current_owner = 0;
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
	moo_status("Task Owner is now: %d", g_current_owner);
	return(g_current_owner);
}

moo_id_t MooTask::current_owner(MooTask *task, moo_id_t id)
{
	if (g_current_task != task)
		return(g_current_owner);
	g_current_owner = id;
	moo_status("Task Owner is now: %d", g_current_owner);
	return(g_current_owner);
}

int MooTask::switch_task(MooTask *task)
{
	g_current_task = task;
	if (task)
		g_current_owner = task->owner();
	moo_status("Task Owner is now: %d", g_current_owner);
	return(0);
}


class TaskEventSuid : public MooCodeEvent {
	MooTask *m_task;
	moo_id_t m_owner;

    public:
	TaskEventSuid(MooTask *task, moo_id_t owner) : MooCodeEvent(NULL, NULL, NULL) {
		m_task = task;
		m_owner = owner;
	}

	int do_event(MooCodeFrame *frame) {
		MooTask::current_owner(m_task, m_owner);
		return(0);
	}
};

int MooTask::suid(MooObject *obj, MooCodeFrame *frame)
{
	frame->push_event(new TaskEventSuid(MooTask::current_task(), MooTask::current_owner()));
	MooTask::current_owner(obj->owner());
	return(0);
}


