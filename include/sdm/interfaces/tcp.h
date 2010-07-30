/*
 * Header Name:	tcp.h
 * Description:	TCP Network Interface Manager Header
 */

#ifndef _SDM_INTERFACES_TCP_H
#define _SDM_INTERFACES_TCP_H

#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>

#define SDM_TCP_CONNECT		1
#define SDM_TCP_LISTEN		2
#define SDM_TCP_ACCEPT		3

#ifndef TCP_READ_BUFFER
#define TCP_READ_BUFFER		512
#endif

class MooTCP : public MooInterface {
	int m_read_pos;
	int m_read_length;
	unsigned char m_read_buffer[TCP_READ_BUFFER];
    public:
	MooTCP();
	virtual ~MooTCP();

	int connect(const char *addr, int port);
	int listen(int port);
	int accept(MooTCP *inter);
	void disconnect();

	virtual int read(char *data, int len);
	virtual int write(const char *data);

	int receive(char *buffer, int size);
	int send(const char *msg, int size);

	int recv_to_buffer();
	int load_buffer(const char *data, int len);
	int read_pos(int pos);
	void clear_buffer();
};

extern MooObjectType moo_tcp_obj_type;

MooObject *moo_tcp_create(void);

#endif

