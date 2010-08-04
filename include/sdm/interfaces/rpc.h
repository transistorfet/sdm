/*
 * Name:	rpc.h
 * Description:	RPC Handling
 */

#ifndef _SDM_INTERFACE_RPC_H
#define _SDM_INTERFACE_RPC_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

class MooRPC : public MooTCP {
    public:
	MooRPC();
	virtual ~MooRPC();

};

extern MooObjectType moo_rpc_obj_type;

MooObject *moo_rpc_create(void);

#endif

