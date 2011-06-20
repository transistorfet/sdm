/*
 * Task Name:	init.h
 * Description:	Init Task
 */

#ifndef _SDM_TASKS_INIT_H
#define _SDM_TASKS_INIT_H

#include <string>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

#include <sdm/objs/object.h>

class MooInit : public MooTask {

    public:
	MooInit();
	~MooInit();

	int read_entry(const char *type, MooDataFile *data);
	int write_data(MooDataFile *data);

	int initialize();
	int idle();
	int release();
};

extern MooObjectType moo_init_obj_type;

MooObject *make_moo_init(MooDataFile *data);

#endif



