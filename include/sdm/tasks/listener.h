/*
 * Task Name:	listener.h
 * Description:	Listener Task
 */

#ifndef _SDM_TASKS_LISTENER_H
#define _SDM_TASKS_LISTENER_H

#include <string>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

#include <sdm/objs/object.h>

class MooListener : public MooTask {
	int m_port;
	const MooObjectType *m_itype;
	const MooObjectType *m_ttype;
	MooTCP *m_inter;

    public:
	MooListener();
	~MooListener();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int initialize();
	int idle();
	int release();

	int handle(MooInterface *inter, int ready);

	int listen(int port);
};

extern MooObjectType moo_listener_obj_type;

MooObject *moo_listener_create(void);

#endif



