/*
 * Module Name:	telnet.c
 * Description:	Telnet Protocol Interface Manager
 */

#include <crypt.h>

#include <sdm/memory.h>

#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>
#include <sdm/interface/telnet.h>

#include <sdm/objs/user.h>

#define TELNET_PORT		4000
#define BUFFER_SIZE		512

#define IS_WHITESPACE(ch)	\
	( ((ch) == ' ') || ((ch) == '\t') )

struct sdm_interface_type sdm_telnet_type = {
	sizeof(struct sdm_tcp),
	(sdm_int_init_t) sdm_tcp_init,
	(sdm_int_release_t) sdm_telnet_release,
	(sdm_int_read_t) sdm_telnet_read,
	(sdm_int_write_t) sdm_telnet_write
};

static struct sdm_tcp *telnet_server;

static int telnet_accept_connection(void *, struct sdm_tcp *);
static int telnet_handle_read(struct sdm_user *, struct sdm_tcp *);
static int telnet_interpret_command(struct sdm_tcp *, const char *, int);

int init_telnet(void)
{
	if (telnet_server)
		return(1);
	if (!(telnet_server = (struct sdm_tcp *) create_sdm_interface(&sdm_telnet_type, SDM_TCP_LISTEN, TELNET_PORT)))
		return(-1);
	sdm_interface_set_callback(SDM_INTERFACE(telnet_server), IO_COND_READ, (callback_t) telnet_accept_connection, NULL);
	return(0);
}

int release_telnet(void)
{
	if (!telnet_server)
		return(1);
	destroy_sdm_interface(SDM_INTERFACE(telnet_server));
	telnet_server = NULL;
	return(0);
}


void sdm_telnet_release(struct sdm_tcp *inter)
{
	struct sdm_object *obj;
	struct callback_s callback;

	callback = sdm_interface_get_callback(SDM_INTERFACE(inter));
	if ((obj = SDM_OBJECT(callback.ptr))) {
		if (sdm_object_is_a(obj, &sdm_user_obj_type) && SDM_USER(obj)->proc)
			sdm_processor_shutdown(SDM_USER(obj)->proc, SDM_USER(obj));
		destroy_sdm_object(obj);
	}
	sdm_tcp_release(inter);
}

int sdm_telnet_read(struct sdm_tcp *inter, char *buffer, int max)
{
	int j = 0;
	int i, res;

	if ((res = sdm_tcp_read_to_buffer(inter)) < 0)
		return(-1);
	/** Ignore leading whitespace */
	for (i = 0; i < res && IS_WHITESPACE(inter->read_buffer[i]); i++)
		;
	for (; i < res; i++) {
		if (inter->read_buffer[i] == 0xff) {
			i++;
			i += telnet_interpret_command(inter, &inter->read_buffer[i], res - i);
		}
		else if ((inter->read_buffer[i] == '\r') || (inter->read_buffer[i] == '\n')) {
			buffer[j] = '\0';
			for (; (i < res) && ((inter->read_buffer[i] == '\r' || (inter->read_buffer[i] == '\n') || (inter->read_buffer[i] == '\0'))); i++)
				;
			sdm_tcp_advance_read_position(inter, i);
			return(j);
		}
		else if ((inter->read_buffer[i] == 0x08) && (j > 0)) {
			j--;
		}
		else if ((inter->read_buffer[i] >= 0x20) && (inter->read_buffer[i] <= 0x7f)) {
			buffer[j++] = inter->read_buffer[i];
		}
	}
	/** We didn't receive a full line so we'll stick this back in the buffer */
	sdm_tcp_set_read_buffer(inter, buffer, j);
	SDM_INTERFACE_SET_NOT_READY_READ(inter);
	return(0);
}

int sdm_telnet_write(struct sdm_tcp *inter, const char *str)
{
	int res;
	char *line;
	int len = 0;

	// TODO rewrite this later to convert colour formatting tags
	while ((*str != '\0') && (line = strchr(str, '\n'))) {
		if ((res = sdm_tcp_write(inter, str, line - str)) < 0)
			return(-1);
		len += res + 1;
		if (sdm_tcp_write(inter, "\r\n", 2) < 0)
			return(-1);
		str = line + 1;
	}
	if (*str != '\0') {
		if ((res = sdm_tcp_write(inter, str, strlen(str))) < 0)
			return(-1);
		len += res;
	}
	return(len);
}


void sdm_telnet_echo(struct sdm_tcp *inter, int action)
{
	if (action) {
		/** Request the other end echo input locally */
		sdm_telnet_write(inter, "\xff\xfc\x01");
	}
	else {
		/** Request the other end stop echoing input locally */
		sdm_telnet_write(inter, "\xff\xfb\x01");
	}
}


void sdm_telnet_encrypt_password(const char *salt, char *buffer, int max)
{
	char *enc;

	enc = crypt(buffer, salt);
	strncpy(buffer, enc, max);
}


int sdm_telnet_run(struct sdm_tcp *inter, struct sdm_user *user)
{
	sdm_processor_startup(user->proc, user);
	sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) telnet_handle_read, user);
	return(0);
}

/*** Local Functions ***/

static int telnet_accept_connection(void *ptr, struct sdm_tcp *server)
{
	struct sdm_tcp *inter;
	if (!(inter = sdm_tcp_accept(&sdm_telnet_type, server)))
		return(0);
	sdm_telnet_login(inter);
	return(0);
}

static int telnet_handle_read(struct sdm_user *user, struct sdm_tcp *inter)
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

static int telnet_interpret_command(struct sdm_tcp *inter, const char *cmd, int len)
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



