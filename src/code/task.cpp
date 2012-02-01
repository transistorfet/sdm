/*
 * Object Name:	task.c
 * Description:	Task Object
 */

#include <stdarg.h>
#include <sys/time.h>

#include <sdm/globals.h>

#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/drivers/driver.h>
#include <sdm/code/code.h>

#include <sdm/code/task.h>

#define TASK_LIST_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE

class MooTaskQueueEntry {
    public:
	MooTaskQueueEntry(double time, MooTask *task) { m_time = time; m_task = task; m_next = NULL; m_prev = NULL; }
	double m_time;
	MooTask *m_task;
	MooTaskQueueEntry *m_next;
	MooTaskQueueEntry *m_prev;
};

class MooTaskQueue {
	MooTaskQueueEntry *m_first;
	MooTaskQueueEntry *m_last;

    public:
	MooTaskQueue();
	~MooTaskQueue();

	int schedule(double time, MooTask *task);
	MooTask *consume();
};

static moo_id_t g_current_owner = -1;
static MooTask *g_current_task = NULL;
static MooArray<MooTask *> *g_task_list = NULL;
static MooTaskQueue *g_task_queue = NULL;

int init_task(void)
{
	if (g_task_list)
		return(1);
	g_task_list = new MooArray<MooTask *>(MOO_ARRAY_DEFAULT_SIZE, -1, TASK_LIST_BITS);
	g_task_queue = new MooTaskQueue();
	g_current_task = NULL;
	g_current_owner = 0;
	return(0);
}

void release_task(void)
{
	delete g_task_queue;
	delete g_task_list;
}

MooTask::MooTask()
{
	this->init(NULL);
}

MooTask::MooTask(MooObjectHash *env)
{
	this->init(env);
}

void MooTask::init(MooObjectHash *env)
{
	m_tid = g_task_list->add(this);
	if (g_current_task)
		m_parent_tid = g_current_task->m_tid;
	else
		m_parent_tid = -1;
	m_frame = new MooCodeFrame(env);
	//this->initialize();
}

MooTask::~MooTask()
{
	delete m_frame;
	m_frame = NULL;
	//this->release();
	if (this == g_current_task) {
		g_current_task = NULL;
		g_current_owner = -1;
	}
	g_task_list->set(m_tid, NULL);
}

int MooTask::run_idle()
{
	int ret = 0;
	MooTask *task;

	while ((task = g_task_queue->consume())) {
		MooTask::switch_task(task);
		if (task->idle())
			ret = 1;
	}
	return(ret);
}

void MooTask::schedule(double time)
{
	g_task_queue->schedule(time, this);
}

int MooTask::initialize()
{

}

int MooTask::idle()
{
	int cycles;
	clock_t start;

	start = clock();
	try {
		cycles = m_frame->run();
	}
	catch (MooException e) {
		// TODO temporary for debugging purposes??
		moo_status("CODE: %s", e.get());
		m_frame->print_stacktrace();
		cycles = -1;

		//m_frame->clear();

		// TODO you need some way of reporting the error back to the user
		// TODO the problem is you need to call basic_print, but it requires a frame and it calls a method, so you need to
		//	either run the frame back to it's starting point (I suppose you could even push a breakpoint or something on
		//	to the event stack rather than make custom provisions in frame->run().)  You need to detect double errors though
		//	where an error occurs in the new code being called so you don't loop infinitely.  You could also somehow package
		//	the event stack into an exception, clear everything, and then push the error reporting code.
		/*
		channel = dynamic_cast<MooThing *>(env->get("channel"));
		if ((thing = MooThing::lookup(MooTask::current_user())))
			thing->notify(TNT_STATUS, NULL, channel, e.get());
		*/
	}
	//moo_status("Executed (%s ...) in %f seconds", name, ((float) clock() - start) / CLOCKS_PER_SEC);
	return(cycles);
}

int MooTask::release()
{

}

int MooTask::push_method_call(const char *name, MooObject *obj, MooObject *arg1, MooObject *arg2, MooObject *arg3)
{
	return(m_frame->push_method_call(name, obj, arg1, arg2, arg3));
}

int MooTask::push_code(const char *code)
{
	return(m_frame->push_code(code));
}

int MooTask::switch_handle(MooDriver *driver, int ready)
{
	MooTask::switch_task(this);
	// TODO we don't have a handler anymore
	//return(this->handle(driver, ready));
	return(0);
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
	return(g_current_task->m_frame->owner());
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
		g_current_owner = task->m_frame->owner();
	moo_status("Task Owner is now: %d", g_current_owner);
	return(0);
}


void *moo_task_worker_thread(void *ptr)
{

	// TODO this thread will constantly try to consume a task from the queue and run it until completion, or until it is suspended


}

/**********************
 * class MooTaskQueue *
 **********************/

MooTaskQueue::MooTaskQueue()
{
	m_first = NULL;
	m_last = NULL;
}

MooTaskQueue::~MooTaskQueue()
{
	MooTaskQueueEntry *cur, *tmp;

	for (cur = m_first; cur; cur = tmp) {
		tmp = cur->m_next;
		delete cur;
	}
}

int MooTaskQueue::schedule(double time, MooTask *task)
{
	timeval tv;
	MooTaskQueueEntry *cur, *prev, *entry;

	gettimeofday(&tv, NULL);
	time = time + tv.tv_sec + (tv.tv_usec / 1000000.0);

	entry = new MooTaskQueueEntry(time, task);
	cur = m_first;
	prev = NULL;
	while (cur && cur->m_time < entry->m_time) {
		prev = cur;
		cur = cur->m_next;
	}

	entry->m_next = cur;
	if (cur)
		cur->m_prev = entry;
	else
		m_last = entry;

	entry->m_prev = prev;
	if (prev)
		prev->m_next = entry;
	else
		m_first = entry;
	return(0);
}

MooTask *MooTaskQueue::consume()
{
	double time;
	timeval tv;
	MooTask *task;
	MooTaskQueueEntry *entry;

	gettimeofday(&tv, NULL);
	time = tv.tv_sec + (tv.tv_usec / 1000000.0);

	// TODO I guess in future this could block until a task is ready, but for now we will return NULL
	if (m_first && m_first->m_time <= time) {
		entry = m_first;
		m_first = entry->m_next;
		if (m_first)
			m_first->m_prev = NULL;
		if (m_last == entry)
			m_last = NULL;
		task = entry->m_task;
		delete entry;
		return(task);
	}
	return(NULL);
}


