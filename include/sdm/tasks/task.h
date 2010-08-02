/*
 * Object Name:	task.h
 * Description:	Task Object
 */

#ifndef _SDM_TASKS_TASK_H
#define _SDM_TASKS_TASK_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef int moo_tid_t;

class MooInterface;

class MooTask : public MooObject {
	moo_tid_t m_tid;
	moo_tid_t m_parent_tid;

    public:
	MooTask();
	virtual ~MooTask();

	virtual int initialize() = 0;
	virtual int idle() = 0;
	virtual int handle(MooInterface *inter, int ready) = 0;
	virtual int release() = 0;
};

int init_task(void);
void release_task(void);
extern MooObjectType moo_task_obj_type;

#endif

