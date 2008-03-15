/*
 * Module Name:	telnet.c
 * Description:	Telnet Protocol Interface Manager
 */

#include <crypt.h>

#include <sdm/hash.h>
#include <sdm/memory.h>

#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>
#include <sdm/interface/telnet.h>

#include <sdm/objs/user.h>

#define TELNET_PORT		4000
#define BUFFER_SIZE		512
#define MAX_ATTRIB		64

#define SDM_ATTR_BOLD		0x01
#define SDM_ATTR_FLASH		0x02
#define SDM_ATTR_UNDERLINE	0x04

#define SDM_ATTR_BLACK		0
#define SDM_ATTR_RED		1
#define SDM_ATTR_GREEN		2
#define SDM_ATTR_YELLOW		3
#define SDM_ATTR_BLUE		4
#define SDM_ATTR_MAGENTA	5
#define SDM_ATTR_CYAN		6
#define SDM_ATTR_WHITE		7

#define IS_WHITESPACE(ch)	\
	( ((ch) == ' ') || ((ch) == '\t') )

struct sdm_telnet_attrib {
	char attrib;
	char fg;
	char bg;
};

struct sdm_interface_type sdm_telnet_type = {
	sizeof(struct sdm_tcp),
	(sdm_int_init_t) sdm_tcp_init,
	(sdm_int_release_t) sdm_telnet_release,
	(sdm_int_read_t) sdm_telnet_read,
	(sdm_int_write_t) sdm_telnet_write
};

static struct sdm_tcp *telnet_server;

static int sdm_telnet_accept_connection(void *, struct sdm_tcp *);
static int sdm_telnet_handle_read(struct sdm_user *, struct sdm_tcp *);
static inline int sdm_telnet_interpret_command(struct sdm_tcp *, const char *, int);
static inline int sdm_telnet_rewrite_attrib(struct sdm_tcp *, char *, struct sdm_telnet_attrib *, int *);

int init_telnet(void)
{
	if (telnet_server)
		return(1);
	if (!(telnet_server = (struct sdm_tcp *) create_sdm_interface(&sdm_telnet_type, SDM_TCP_LISTEN, TELNET_PORT)))
		return(-1);
	sdm_interface_set_callback(SDM_INTERFACE(telnet_server), IO_COND_READ, (callback_t) sdm_telnet_accept_connection, NULL);
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
			i += sdm_telnet_interpret_command(inter, &inter->read_buffer[i], res - i);
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
	int i, j, k;
	int attr_sp = 0;
	char buffer[STRING_SIZE];
	struct sdm_telnet_attrib attribs[MAX_ATTRIB];

	for (i = 0, j = 0; (j < STRING_SIZE) && (str[i] != '\0'); i++) {
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
			for (; (j < STRING_SIZE) && (str[i] != '\0'); i++, j++) {
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
			j = k + sdm_telnet_rewrite_attrib(inter, &buffer[k], attribs, &attr_sp);
			break;
		    }
		    case '&': {
			if (!strncmp(&str[i + 1], "lt;", 3))
				buffer[j++] = '<';
			else if (!strncmp(&str[i + 1], "gt;", 3))
				buffer[j++] = '>';
			else
				buffer[j++] = '&';
			break;
		    }
		    default: {
			buffer[j++] = str[i];
			break;
		    }
		}
	}

	/** If we haven't closed all attribute tags then send the reset escape to put us back to normal */
	if (attr_sp) {
		if (j + 3 < STRING_SIZE) {
			strncpy(&buffer[j], "\x1b[m", 3);
			j += 3;
		}
		else {
			strncpy(&buffer[STRING_SIZE - 3], "\x1b[m", 3);
			j = STRING_SIZE;
		}
	}
	if (sdm_tcp_send(inter, buffer, j) < 0)
		return(-1);
	return(j);
}


void sdm_telnet_echo(struct sdm_tcp *inter, int action)
{
	if (action) {
		/** Request the other end echo input locally */
		sdm_tcp_send(inter, "\xff\xfc\x01", 3);
	}
	else {
		/** Request the other end stop echoing input locally */
		sdm_tcp_send(inter, "\xff\xfb\x01", 3);
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
	sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) sdm_telnet_handle_read, user);
	return(0);
}

/*** Local Functions ***/

static int sdm_telnet_accept_connection(void *ptr, struct sdm_tcp *server)
{
	struct sdm_tcp *inter;
	if (!(inter = sdm_tcp_accept(&sdm_telnet_type, server)))
		return(0);
	sdm_telnet_login(inter);
	return(0);
}

static int sdm_telnet_handle_read(struct sdm_user *user, struct sdm_tcp *inter)
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

static inline int sdm_telnet_interpret_command(struct sdm_tcp *inter, const char *cmd, int len)
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

static inline int sdm_telnet_rewrite_attrib(struct sdm_tcp *inter, char *buffer, struct sdm_telnet_attrib *attrib, int *attr_sp)
{
	// TODO implement this properly
	if (buffer[1] == '/') {
		strncpy(buffer, "\x1b[0m", 4);
		return(4);
	}
	else if (!strcmp(&buffer[1], "b")) {
		strncpy(buffer, "\x1b[1m", 4);
		return(4);
	}
	return(0);
}


/*
	{ "b",			{ SDM_ATTR_BOLD, 0, 0 } },
	{ "black",		{ 0, SDM_ATTR_BLACK, 0 } },
	{ "blue",		{ 0, SDM_ATTR_BLUE, 0 } },
	{ "brightblue",		{ SDM_ATTR_BOLD, SDM_ATTR_BLUE, 0 } },
	{ "brightcyan",		{ SDM_ATTR_BOLD, SDM_ATTR_CYAN, 0 } },
	{ "brightgreen",	{ SDM_ATTR_BOLD, SDM_ATTR_GREEN, 0 } },
	{ "brightmagenta",	{ SDM_ATTR_BOLD, SDM_ATTR_MAGENTA, 0 } },
	{ "brightred",		{ SDM_ATTR_BOLD, SDM_ATTR_RED, 0 } },
	{ "brightwhite",	{ SDM_ATTR_BOLD, SDM_ATTR_WHITE, 0 } },
	{ "brightyellow",	{ SDM_ATTR_BOLD, SDM_ATTR_YELLOW, 0 } },
	{ "cyan",		{ 0, SDM_ATTR_CYAN, 0 } },
	{ "f",			{ SDM_ATTR_FLASH, 0, 0 } },
	{ "green",		{ 0, SDM_ATTR_GREEN, 0 } },
	{ "magenta",		{ 0, SDM_ATTR_MAGENTA, 0 } },
	{ "red",		{ 0, SDM_ATTR_RED, 0 } },
	{ "u",			{ SDM_ATTR_UNDERLINE, 0, 0 } },
	{ "white",		{ 0, SDM_ATTR_WHITE, 0 } },
	{ "yellow",		{ 0, SDM_ATTR_YELLOW, 0 } },
	{ NULL,			{ 0, 0, 0 } }
*/


