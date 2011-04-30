/*
 * Header Name:	tcp.h
 * Description:	TCP Network Interface Manager Header
 */

#ifndef _SDM_INTERFACES_TCP_H
#define _SDM_INTERFACES_TCP_H

#include <string>
#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>

#define SDM_TCP_CONNECT		1
#define SDM_TCP_LISTEN		2
#define SDM_TCP_ACCEPT		3

#ifndef TCP_READ_BUFFER
#define TCP_READ_BUFFER		512
#endif

class MooTCP : public MooInterface {
    protected:
	int m_read_pos;
	int m_read_length;
	char m_read_buffer[TCP_READ_BUFFER];
	std::string *m_host;

    public:
	MooTCP();
	virtual ~MooTCP();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int connect(const char *addr, int port);
	int listen(int port);
	int accept(MooTCP *inter);
	void disconnect();
	const char *host() { if (!m_host) return(NULL); return(m_host->c_str()); }

	virtual int read(char *data, int len);
	virtual int write(const char *data);

	int receive(char *data, int len);
	int receive(char *data, int len, char delim);
	int send(const char *data, int len = -1);

    protected:
	int recv_to_buffer();
	int load_buffer(const char *data, int len);
	int read_pos(int pos);
	void clear_buffer();
};

extern MooObjectType moo_tcp_obj_type;

MooObject *make_moo_tcp(MooDataFile *data);

#endif

