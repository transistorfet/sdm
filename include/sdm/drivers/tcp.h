/*
 * Header Name:	tcp.h
 * Description:	TCP Driver Header
 */

#ifndef _SDM_DRIVERS_TCP_H
#define _SDM_DRIVERS_TCP_H

#include <string>
#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/drivers/driver.h>

#ifndef TCP_READ_BUFFER
#define TCP_READ_BUFFER		512
#endif

class MooTCP : public MooDriver {
    protected:
	int m_read_pos;
	int m_read_length;
	char m_read_buffer[TCP_READ_BUFFER];
	std::string *m_host;
	MooTask *m_task;

    public:
	MooTCP();
	virtual ~MooTCP();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	// TODO add function 'connected()'

	int connect(const char *addr, int port);
	void disconnect();
	const char *host() { if (!m_host) return(NULL); return(m_host->c_str()); }
	int check_ready(int ready);

	virtual int read(char *data, int len);
	virtual int write(const char *data);
	virtual int handle(int ready);

	int receive(char *data, int len);
	int receive(char *data, int len, char delim);
	int send(const char *data, int len = -1);

	int wait_for_data();

    private:
	//virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

    protected:
	int recv_to_buffer();
	int load_buffer(const char *data, int len);
	int read_pos(int pos);
	void clear_buffer();
};

extern MooObjectType moo_tcp_obj_type;

MooObject *load_moo_tcp(MooDataFile *data);

int init_tcp(void);
void release_tcp(void);
void moo_load_tcp_methods(MooObjectHash *env);

#endif

