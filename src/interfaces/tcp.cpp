/*
 * Module Name:		tcp.c
 * System Requirements:	Unix Sockets
 * Description:		TCP Network Interface Manager
 */


#include <string.h>
#include <stdarg.h>

#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

#ifndef TCP_CONNECT_ATTEMPTS
#define TCP_CONNECT_ATTEMPTS		3
#endif

#ifndef TCP_LISTEN_QUEUE
#define TCP_LISTEN_QUEUE		5
#endif

MooObjectType moo_tcp_obj_type = {
	NULL,
	"tcp",
	(moo_type_create_t) moo_tcp_create
};

MooObject *moo_tcp_create(void)
{
	return(new MooTCP());
}


static int tcp_connect(struct sdm_tcp *, const char *, int);
static int tcp_listen(struct sdm_tcp *, int);
static int tcp_accept(struct sdm_tcp *, struct sdm_tcp *);

int sdm_tcp_init(struct sdm_tcp *inter, int nargs, va_list va)
{
	int type;

	if (sdm_interface_init(SDM_INTERFACE(inter), 0, va) < 0)
		return(-1);
	inter->read_pos = 0;
	inter->read_length = 0;
	memset(inter->read_buffer, '\0', TCP_READ_BUFFER);

	type = va_arg(va, int);
	switch (type) {
		case SDM_TCP_CONNECT: {
			const char *server;
			int port;

			server = va_arg(va, const char *);
			port = va_arg(va, int);
			return(tcp_connect(inter, server, port));
		}
		case SDM_TCP_LISTEN: {
			int port;

			port = va_arg(va, int);
			return(tcp_listen(inter, port));
		}
		case SDM_TCP_ACCEPT: {
			struct sdm_tcp *listener;

			listener = va_arg(va, struct sdm_tcp *);
			return(tcp_accept(inter, listener));
			break;
		}
		default:
			break;
	}
	return(-1);
}

MooTCP::~MooTCP()
{
	shutdown(m_rfd, 2);
	close(m_rfd);
	this->rfd = -1;
}


int MooTCP::read(char *data, int len)
{
	int res;

	if ((res = this->receive(data, len - 1)) < 0)
		return(res);
	data[res] = '\0';
	return(res);
}

int MooTCP::write(const char *data)
{
	return(this->send(data, strlen(data)));
}


/**
 * Receive the given number of bytes, store them in the given buffer
 * and return the number of bytes read or -1 on error or disconnect.
 */ 
int MooTCP::receive(char *data, int len)
{
	int i, j;
	fd_set rd;
	struct timeval timeout = { 0, 0 };

	this->set_not_ready();
	for (i = 0;i < size;i++) {
		if (m_read_pos >= m_read_length)
			break;
		buffer[i] = m_read_buffer[m_read_pos++];
	}

	if (i < size) {
		FD_ZERO(&rd);
		FD_SET(m_rfd, &rd);
		if (select(m_rfd + 1, &rd, NULL, NULL, &timeout)
		    && ((j = recv(m_rfd, &buffer[i], size - i, 0)) > 0))
			i += j;
		if (j <= 0)
			return(-1);
	}
	if (m_read_pos < m_read_length)
		this->set_ready();
	return(i);
}

/**
 * Send the string of length size to the given network connection and
 * return the number of bytes written or -1 on error.
 */
int MooTCP::send(const char *msg, int size)
{
	int sent, count = 0;

	do {
		if ((sent = send(m_rfd, (void *) msg, size, 0)) < 0)
			return(-1);
		else if (!sent)
			return(0);
		count += sent;
	} while (count < size);
	return(count);
}


/**
 * Receive data directly into the read buffer and return the number of bytes
 * read or -1 on error or disconnect.
 */ 
int sdm_tcp_read_to_buffer(struct sdm_tcp *inter)
{
	int res;
	fd_set rd;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&rd);
	FD_SET(SDM_INTERFACE(inter)->read, &rd);
	if ((res = select(SDM_INTERFACE(inter)->read + 1, &rd, NULL, NULL, &timeout)) < 0)
		return(-1);
	else if (res == 0)
		return(inter->read_length - inter->read_pos);

	if ((res = recv(SDM_INTERFACE(inter)->read, &inter->read_buffer[inter->read_length], TCP_READ_BUFFER - inter->read_length - 1, 0)) <= 0)
		return(-1);
	res += inter->read_length;
	inter->read_length = res;
	inter->read_buffer[res] = '\0';
	SDM_INTERFACE_SET_READY_READ(inter);
	return(res - inter->read_pos);
}

/**
 * Set the read buffer to contain the given data.
 */
int sdm_tcp_set_read_buffer(struct sdm_tcp *inter, const char *buffer, int len)
{
	if (len > TCP_READ_BUFFER)
		len = TCP_READ_BUFFER;
	strncpy(inter->read_buffer, buffer, len);
	inter->read_length = len;
	inter->read_pos = 0;
	if (len)
		SDM_INTERFACE_SET_READY_READ(inter);
	else
		SDM_INTERFACE_SET_NOT_READY_READ(inter);
	return(len);
}

/**
 * Set the position in the read buffer that has been read to the given value or
 * if the position is greater than or equal to the length of the read buffer,
 * clear the read buffer.  The position of the read pointer is returned.
 */
int sdm_tcp_advance_read_position(struct sdm_tcp *inter, int pos)
{
	if (pos >= inter->read_length) {
		SDM_INTERFACE_SET_NOT_READY_READ(inter);
		inter->read_pos = 0;
		inter->read_length = 0;
	}
	else {
		SDM_INTERFACE_SET_READY_READ(inter);
		inter->read_pos = pos;
	}
	return(inter->read_pos);
}

/**
 * Clear the read buffer.
 */
void sdm_tcp_clear_buffer(struct sdm_tcp *inter)
{
	SDM_INTERFACE_SET_NOT_READY_READ(inter);
	inter->read_pos = 0;
	inter->read_length = 0;
}


/*** Local Functions ***/

static int tcp_connect(struct sdm_tcp *inter, const char *server, int port)
{
	int i, j;
	struct hostent *host;
	struct sockaddr_in saddr;

	if (!(host = gethostbyname(server)))
		return(-1);
	memset(&saddr, '\0', sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	if ((SDM_INTERFACE(inter)->read = socket(PF_INET, SOCK_STREAM, 0)) >= 0) {
		for (j = 0;host->h_addr_list[j];j++) {
			saddr.sin_addr = *((struct in_addr *) host->h_addr_list[j]);
			for (i = 0;i < TCP_CONNECT_ATTEMPTS;i++) {
				if (connect(SDM_INTERFACE(inter)->read, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
					return(0);
			}
		}
	}
	return(-1);
}

static int tcp_listen(struct sdm_tcp *inter, int port)
{
	struct sockaddr_in saddr;

	memset(&saddr, '\0', sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	if (((SDM_INTERFACE(inter)->read = socket(PF_INET, SOCK_STREAM, 0)) >= 0)
	    && (bind(SDM_INTERFACE(inter)->read, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) >= 0)
	    && (listen(SDM_INTERFACE(inter)->read, TCP_LISTEN_QUEUE) >= 0)) {
		return(0);
	}
	return(-1);
}

static int tcp_accept(struct sdm_tcp *inter, struct sdm_tcp *listener)
{
	int size;
	fd_set rd;
	struct sockaddr_in saddr;
	struct timeval timeout = { 0, 0 };

	/** Make sure there is a connection waiting */
	FD_ZERO(&rd);
	FD_SET(SDM_INTERFACE(listener)->read, &rd);
	if (select(SDM_INTERFACE(listener)->read + 1, &rd, NULL, NULL, &timeout) <= 0)
		return(-1);

	size = sizeof(struct sockaddr_in);
	if ((SDM_INTERFACE(inter)->read = accept(SDM_INTERFACE(listener)->read, (struct sockaddr *) &saddr, &size))) {
		sdm_status("Accepted connection from %s", inet_ntoa(saddr.sin_addr));
		// TODO do anything you might want with the saddr
		return(0);
	}
	return(-1);
}


