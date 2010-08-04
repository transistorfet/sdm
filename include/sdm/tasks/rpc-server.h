/*
 * Task Name:	rpc-server.h
 * Description:	RPC Server Task
 */

#ifndef _SDM_TASKS_RPC_SERVER_H
#define _SDM_TASKS_RPC_SERVER_H

#include <string>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>

class MooRPCServer : public MooTask {
	int m_port;
	const MooObjectType *m_itype;
	const MooObjectType *m_ttype;
	MooTCP *m_inter;

    public:
	MooRPCServer();
	~MooRPCServer();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int initialize();
	int idle();
	int release();

	int handle(MooInterface *inter, int ready);
	int bestow(MooInterface *inter);
};

extern MooObjectType moo_rpc_server_obj_type;

MooObject *moo_rpc_server_create(void);

#endif


