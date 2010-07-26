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

	int connect(const char *server, const char *port = NULL);
	int accept();
	void disconnect();

	virtual int read(char *data, int len);
	virtual int write(const char *data);

	int receive(char *buffer, int size);
	int send(const char *msg, int size);
};

extern MooObjectType moo_tcp_obj_type;

MooObject *moo_tcp_create(void);

int sdm_tcp_init(struct sdm_tcp *, int, va_list);
void sdm_tcp_release(struct sdm_tcp *);

int sdm_tcp_read(struct sdm_tcp *, char *, int);
int sdm_tcp_write(struct sdm_tcp *, const char *);

int sdm_tcp_receive(struct sdm_tcp *, char *, int);
int sdm_tcp_send(struct sdm_tcp *, const char *, int);

int sdm_tcp_read_to_buffer(struct sdm_tcp *);
int sdm_tcp_set_read_buffer(struct sdm_tcp *, const char *, int);
int sdm_tcp_advance_read_position(struct sdm_tcp *, int);
void sdm_tcp_clear_buffer(struct sdm_tcp *);

#endif

