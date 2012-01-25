/*
 * Header Name:	server.h
 * Description:	TCP Server Driver Header
 */

#ifndef _SDM_DRIVERS_SERVER_H
#define _SDM_DRIVERS_SERVER_H

#include <string>
#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/drivers/driver.h>

class MooServer : public MooDriver {
    protected:
	int m_port;

    public:
	MooServer();
	virtual ~MooServer();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int listen(int port);
	MooTCP *MooServer::receive_connection();
	void disconnect();
};

extern MooObjectType moo_server_obj_type;

MooObject *load_moo_server(MooDataFile *data);

#endif

