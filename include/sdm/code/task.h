/*
 * Object Name:	task.h
 * Description:	Task Object
 */

#ifndef _SDM_TASKS_TASK_H
#define _SDM_TASKS_TASK_H

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define TNT_STATUS	0x01
#define TNT_JOIN	0x02
#define TNT_LEAVE	0x03
#define TNT_SAY		0x04
#define TNT_EMOTE	0x05
#define TNT_QUIT	0x06

#define TNT_FIRST	0x01
#define TNT_LAST	0x06

typedef int moo_tid_t;

class MooTaskQueueEntry {
    public:
	MooTaskQueueEntry(double time, MooCodeFrame *frame) { m_time = time; m_frame = frame; m_next = NULL; m_prev = NULL; }
	double m_time;
	MooCodeFrame *m_frame;
	MooTaskQueueEntry *m_next;
	MooTaskQueueEntry *m_prev;
};

class MooTaskQueue {
	MooTaskQueueEntry *m_first;
	MooTaskQueueEntry *m_last;

    public:
	MooTaskQueue();
	~MooTaskQueue();

	int schedule(MooCodeFrame *frame, double time);
	MooCodeFrame *consume();
};

#endif


