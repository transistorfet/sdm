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
	MooMutable *m_obj;
	std::string *m_method;

    public:
	MooServer();
	virtual ~MooServer();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int handle(int ready);

	int listen(int port);
	MooTCP *receive_connection();
	void disconnect();

	int set_callback(MooMutable *obj, const char *method);

    private:
	//virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);
};

extern MooObjectType moo_server_obj_type;
MooObject *load_moo_server(MooDataFile *data);

int init_server(void);
void release_server(void);

#endif

