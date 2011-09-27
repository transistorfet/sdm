/*
 * Task Name:	code.h
 * Description:	Code Task
 */

#ifndef _SDM_TASKS_CODE_H
#define _SDM_TASKS_CODE_H

#include <string>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

#include <sdm/objs/object.h>
#include <sdm/code/code.h>

class MooCodeTask : public MooTask {
	MooObjectHash *m_env;
	MooCodeFrame *m_frame;

    public:
	MooCodeTask();
	MooCodeTask(MooThing *user, MooThing *channel);
	~MooCodeTask();

	int read_entry(const char *type, MooDataFile *data);
	int write_data(MooDataFile *data);

	int initialize();
	int idle();
	int release();

	int handle(MooInterface *inter, int ready);
	int purge(MooInterface *inter);

	int push_call(MooObject *func, MooObject *arg1 = NULL, MooObject *arg2 = NULL, MooObject *arg3 = NULL);
	int push_code(const char *code);
};

extern MooObjectType moo_code_task_obj_type;

MooObject *make_moo_code_task(MooDataFile *data);

#endif



