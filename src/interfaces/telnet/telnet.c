/*
 * Module Name:	telnet.c
 * Description:	Telnet Protocol Interface Manager
 */

#include <crypt.h>

#include <sdm/hash.h>
#include <sdm/memory.h>

#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/telnet.h>
#include "ansi.h"

#include <sdm/objs/user.h>

#define TELNET_PORT		4000
#define BUFFER_SIZE		512
#define MAX_ATTRIB		64

#define IS_WHITESPACE(ch)	\
	( ((ch) == ' ') || ((ch) == '\t') )

struct sdm_interface_type sdm_telnet_type = {
	sizeof(struct sdm_telnet),
	(sdm_int_init_t) sdm_tcp_init,
	(sdm_int_release_t) sdm_telnet_release,
	(sdm_int_read_t) sdm_telnet_read,
	(sdm_int_write_t) sdm_telnet_write
};

static struct sdm_telnet *telnet_server = NULL;

static int sdm_telnet_accept_connection(void *, struct sdm_telnet *);
static int sdm_telnet_handle_read(struct sdm_user *, struct sdm_telnet *);
static inline int sdm_telnet_interpret_command(struct sdm_telnet *, const char *, int);

int init_telnet(void)
{
	if (telnet_server)
		return(1);
	if (!(telnet_server = (struct sdm_telnet *) create_sdm_interface(&sdm_telnet_type, SDM_TCP_LISTEN, TELNET_PORT)))
		return(-1);
	if (init_telnet_ansi() < 0)
		return(-1);
	sdm_interface_set_callback(SDM_INTERFACE(telnet_server), IO_COND_READ, (callback_t) sdm_telnet_accept_connection, NULL);
	return(0);
}

int release_telnet(void)
{
	if (!telnet_server)
		return(1);
	release_telnet_ansi();
	destroy_sdm_interface(SDM_INTERFACE(telnet_server));
	telnet_server = NULL;
	return(0);
}


void sdm_telnet_release(struct sdm_telnet *inter)
{
	struct sdm_object *obj;
	struct callback_s callback;

	callback = sdm_interface_get_callback(SDM_INTERFACE(inter));
	if ((obj = SDM_OBJECT(callback.ptr))) {
		if (sdm_object_is_a(obj, &sdm_user_obj_type) && SDM_USER(obj)->proc)
			sdm_processor_shutdown(SDM_USER(obj)->proc, SDM_USER(obj));
		destroy_sdm_object(obj);
	}
	sdm_tcp_release(SDM_TCP(inter));
}

int sdm_telnet_read(struct sdm_telnet *inter, char *buffer, int max)
{
	int j = 0;
	int i, res;

	if ((res = sdm_tcp_read_to_buffer(SDM_TCP(inter))) < 0)
		return(-1);
	/** Ignore leading whitespace */
	for (i = 0; i < res && IS_WHITESPACE(SDM_TCP(inter)->read_buffer[i]); i++)
		;
	for (; i < res; i++) {
		if (SDM_TCP(inter)->read_buffer[i] == 0xff) {
			i++;
			i += sdm_telnet_interpret_command(inter, &SDM_TCP(inter)->read_buffer[i], res - i);
		}
		else if ((SDM_TCP(inter)->read_buffer[i] == '\r') || (SDM_TCP(inter)->read_buffer[i] == '\n')) {
			buffer[j] = '\0';
			for (; (i < res) && ((SDM_TCP(inter)->read_buffer[i] == '\r' || (SDM_TCP(inter)->read_buffer[i] == '\n') || (SDM_TCP(inter)->read_buffer[i] == '\0'))); i++)
				;
			sdm_tcp_advance_read_position(SDM_TCP(inter), i);
			return(j);
		}
		else if ((SDM_TCP(inter)->read_buffer[i] == 0x08) && (j > 0)) {
			j--;
		}
		else if ((SDM_TCP(inter)->read_buffer[i] >= 0x20) && (SDM_TCP(inter)->read_buffer[i] <= 0x7f)) {
			buffer[j++] = SDM_TCP(inter)->read_buffer[i];
		}
	}
	/** We didn't receive a full line so we'll stick this back in the buffer */
	sdm_tcp_set_read_buffer(SDM_TCP(inter), buffer, j);
	SDM_INTERFACE_SET_NOT_READY_READ(inter);
	return(0);
}

int sdm_telnet_write(struct sdm_telnet *inter, const char *str)
{
	int res;
	int i, j, k;
	char buffer[LARGE_STRING_SIZE];
	struct sdm_telnet_attrib_stack stack;

	stack.sp = 0;
	for (i = 0, j = 0; (j < LARGE_STRING_SIZE) && (str[i] != '\0'); i++) {
		switch (str[i]) {
		    case '\n': {
			buffer[j++] = '\r';
			buffer[j++] = '\n';
			break;
		    }
		    case '<': {
			k = j;
			buffer[j++] = str[i++];
			if (str[i] == '/')
				buffer[j++] = str[i++];
			for (; (j < LARGE_STRING_SIZE) && (str[i] != '\0'); i++, j++) {
				/** Break at the first non-alphanumeric or '_' character. */
				if (!(((str[i] >= '0') && (str[i] <= '9'))
				    || ((str[i] >= 'A') && (str[i] <= 'Z'))
				    || ((str[i] >= 'a') && (str[i] <= 'z')) || (str[i] == '_')))
					break;
				buffer[j] = str[i];
			}
			if (str[i] != '>') {
				/** This wasn't a tag so we'll ignore it */
				buffer[j++] = str[i];
				break;
			}
			buffer[j] = '\0';
			j = k + sdm_telnet_write_attrib(inter, &buffer[k + 1], &buffer[k], LARGE_STRING_SIZE - k - 1, &stack);
			break;
		    }
		    case '&': {
			if (!strncmp(&str[i + 1], "lt;", 3)) {
				buffer[j++] = '<';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "gt;", 3)) {
				buffer[j++] = '>';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "amp;", 4)) {
				buffer[j++] = '&';
				i += 4;
			}
			else if (!strncmp(&str[i + 1], "quot;", 5)) {
				buffer[j++] = '\"';
				i += 5;
			}
			else
				buffer[j++] = '&';
			break;
		    }
		    default: {
			if (str[i] >= 0x20)
				buffer[j++] = str[i];
			break;
		    }
		}
	}

	res = sdm_tcp_send(SDM_TCP(inter), buffer, j);
	/** If we haven't closed all attribute tags then send the reset escape to put us back to normal */
	if (stack.sp != 0)
		sdm_telnet_reset_attribs(inter);
	if (res < 0)
		return(-1);
	return(j);
}


void sdm_telnet_echo(struct sdm_telnet *inter, int action)
{
	if (action) {
		/** Request the other end echo input locally */
		sdm_tcp_send(SDM_TCP(inter), "\xff\xfc\x01", 3);
	}
	else {
		/** Request the other end stop echoing input locally */
		sdm_tcp_send(SDM_TCP(inter), "\xff\xfb\x01", 3);
	}
}


void sdm_telnet_encrypt_password(const char *salt, char *buffer, int max)
{
	char *enc;

	enc = crypt(buffer, salt);
	strncpy(buffer, enc, max);
}


int sdm_telnet_run(struct sdm_telnet *inter, struct sdm_user *user)
{
	sdm_processor_startup(user->proc, user);
	sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) sdm_telnet_handle_read, user);
	return(0);
}

/*** Local Functions ***/

static int sdm_telnet_accept_connection(void *ptr, struct sdm_telnet *server)
{
	struct sdm_telnet *inter;
	if (!(inter = SDM_TELNET(sdm_tcp_accept(&sdm_telnet_type, server))))
		return(0);
	sdm_telnet_login(inter);
	return(0);
}

static int sdm_telnet_handle_read(struct sdm_user *user, struct sdm_telnet *inter)
{
	int i;
	char buffer[BUFFER_SIZE];

	if ((i = sdm_telnet_read(inter, buffer, BUFFER_SIZE - 1)) < 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}
	if (i == 0)
		return(0);

	if (sdm_processor_process(user->proc, user, buffer) != 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(1);
	}
	return(0);
}

static inline int sdm_telnet_interpret_command(struct sdm_telnet *inter, const char *cmd, int len)
{
	// TODO do this properly
	switch (cmd[0]) {
	    case TELNET_CMD_WILL:
	    case TELNET_CMD_WONT:
	    case TELNET_CMD_DO:
	    case TELNET_CMD_DONT: {
		return(2);
	    }
	    default: {
		return(1);
	    }
	}
}


