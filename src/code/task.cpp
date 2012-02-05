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

int MooTaskQueue::schedule(MooCodeFrame *frame, double time)
{
	timeval tv;
	MooTaskQueueEntry *cur, *prev, *entry;

	gettimeofday(&tv, NULL);
	time = time + tv.tv_sec + (tv.tv_usec / 1000000.0);

	entry = new MooTaskQueueEntry(time, frame);
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

MooCodeFrame *MooTaskQueue::consume()
{
	double time;
	timeval tv;
	MooCodeFrame *frame;
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
		frame = entry->m_frame;
		delete entry;
		return(frame);
	}
	return(NULL);
}


