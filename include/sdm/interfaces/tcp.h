/*
 * Header Name:	tcp.h
 * Description:	TCP Network Interface Manager Header
 */

#ifndef _SDM_INTERFACE_TCP_H
#define _SDM_INTERFACE_TCP_H

#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/interfaces/interface.h>

#define SDM_TCP_CONNECT		1
#define SDM_TCP_LISTEN		2
#define SDM_TCP_ACCEPT		3

#ifndef TCP_READ_BUFFER
#define TCP_READ_BUFFER		512
#endif

#define SDM_TCP(ptr)		( (struct sdm_tcp *) (ptr) )

struct sdm_tcp {
	struct sdm_interface interface;
	int read_pos;
	int read_length;
	unsigned char read_buffer[TCP_READ_BUFFER];
};

extern struct sdm_interface_type sdm_tcp_type;

#define sdm_tcp_connect(server, port)	\
	( (struct sdm_tcp *) create_sdm_interface(&sdm_tcp_type, SDM_TCP_CONNECT, (server), (port)) )

#define sdm_tcp_listen(port)	\
	( (struct sdm_tcp *) create_sdm_interface(&sdm_tcp_type, SDM_TCP_LISTEN, (port)) )

#define sdm_tcp_accept(type, inter)	\
	( (struct sdm_tcp *) create_sdm_interface((type), SDM_TCP_ACCEPT, (inter)) )

int sdm_tcp_init(struct sdm_tcp *, va_list);
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

