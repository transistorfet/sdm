/*
 * Object Name:	process.h
 * Description:	Process Object
 */

#ifndef _SDM_PROCESSES_PROCESS_H
#define _SDM_PROCESSES_PROCESS_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef int moo_pid_t;

class MooInterface;

class MooProcess : public MooObject {
	moo_pid_t m_pid;
	moo_pid_t m_parent_pid;

    public:
	MooProcess();
	virtual ~MooProcess();

	virtual int initialize() = 0;
	virtual int idle() = 0;
	virtual int handle(MooInterface *inter) = 0;
	virtual int release() = 0;
};

int init_process(void);
void release_process(void);
extern MooObjectType moo_process_obj_type;

MooObject *moo_process_create(void);

#endif

