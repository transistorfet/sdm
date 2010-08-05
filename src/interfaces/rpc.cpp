/*
 * Module Name:	rpc.c
 * Description:	RPC Interface
 */

#include <string.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/rpc.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

MooObjectType moo_rpc_obj_type = {
	&moo_tcp_obj_type,
	"rpc",
	typeid(MooRPC).name(),
	(moo_type_create_t) moo_rpc_create
};

MooObject *moo_rpc_create(void)
{
	return(new MooRPC());
}

MooRPC::MooRPC()
{

}

MooRPC::~MooRPC()
{

}



