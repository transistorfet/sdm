/*
 * Module Name:	login.c
 * Description:	Telnet Login Handler
 */

#include <string.h>

#include <sdm/text.h>
#include <sdm/misc.h>
#include <sdm/globals.h>

#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/telnet.h>

#include <sdm/objs/login.h>
#include <sdm/objs/user.h>

static int sdm_telnet_login_read_name(struct sdm_login *, struct sdm_telnet *);
static int sdm_telnet_login_read_password(struct sdm_login *, struct sdm_telnet *);

int sdm_telnet_login(struct sdm_telnet *inter)
{
	struct sdm_login *login;

	if (!(login = create_sdm_login())) {
		sdm_telnet_write(inter, SDM_TXT_OUT_OF_MEMORY);
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}

	// TODO print welcome screen
	sdm_telnet_write(inter, "\n");
	sdm_telnet_write(inter, SDM_TXT_LOGIN_PROMPT);
	sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) sdm_telnet_login_read_name, login);
	return(0);
}

/*** Local Functions ***/

static int sdm_telnet_login_read_name(struct sdm_login *login, struct sdm_telnet *inter)
{
	int res;
	char buffer[STRING_SIZE];

	if ((res = sdm_telnet_read(inter, buffer, STRING_SIZE - 1)) < 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}
	else if (res == 0)
		return(0);

	if (!strcmp(buffer, "register")) {
		sdm_telnet_register(inter, login);
	}
	else if (!sdm_user_valid_username(buffer)) {
		sdm_telnet_write(inter, SDM_TXT_INVALID_USERNAME);
		sdm_telnet_write(inter, SDM_TXT_LOGIN_PROMPT);
	}
	else if (sdm_login_set_name(login, buffer)) {
		sdm_telnet_write(inter, SDM_TXT_OUT_OF_MEMORY);
		destroy_sdm_interface(SDM_INTERFACE(inter));
	}
	else {
		sdm_telnet_echo(inter, 0);
		sdm_telnet_write(inter, SDM_TXT_PASSWORD_PROMPT);
		sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) sdm_telnet_login_read_password, login);
	}
	return(0);
}

static int sdm_telnet_login_read_password(struct sdm_login *login, struct sdm_telnet *inter)
{
	int res;
	struct sdm_user *user;
	char buffer[STRING_SIZE];

	if ((res = sdm_telnet_read(inter, buffer, STRING_SIZE - 1)) < 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}
	else if (res == 0)
		return(0);

	sdm_telnet_echo(inter, 1);
	sdm_telnet_write(inter, "\n");
	sdm_telnet_encrypt_password(login->name, buffer, STRING_SIZE);
	if (sdm_login_authenticate(login, buffer)) {
		if (sdm_user_logged_in(login->name)) {
			sdm_telnet_write(inter, SDM_TXT_ALREADY_LOGGED_IN);
			destroy_sdm_interface(SDM_INTERFACE(inter));
			return(-1);
		}
		else if (!sdm_user_exists(login->name)) {
			sdm_telnet_write(inter, SDM_TXT_INVALID_USER);
			destroy_sdm_interface(SDM_INTERFACE(inter));
			return(-1);
		}
		else if (!(user = create_sdm_user(login->name, SDM_INTERFACE(inter)))) {
			sdm_telnet_write(inter, SDM_TXT_OUT_OF_MEMORY);
			destroy_sdm_interface(SDM_INTERFACE(inter));
			return(-1);
		}
		else {
			destroy_sdm_object(SDM_OBJECT(login));
			sdm_telnet_write(inter, "\n");
			sdm_telnet_run(inter, user);
		}
	}
	else  {
		sdm_telnet_write(inter, SDM_TXT_WRONG_USER_OR_PASSWORD);
		sdm_telnet_write(inter, SDM_TXT_LOGIN_PROMPT);
		sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) sdm_telnet_login_read_name, login);
	}
	return(0);
}


