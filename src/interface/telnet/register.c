/*
 * Module Name:	register.c
 * Description:	Telnet Character Registration Handler
 */

#include <string.h>

#include <sdm/text.h>
#include <sdm/misc.h>
#include <sdm/globals.h>

#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>
#include <sdm/interface/telnet.h>

#include <sdm/objs/login.h>
#include <sdm/objs/user.h>

static int telnet_register_read_name(struct sdm_login *, struct sdm_tcp *);
static int telnet_register_read_password(struct sdm_login *, struct sdm_tcp *);
static int telnet_register_verify_password(struct sdm_login *, struct sdm_tcp *);

int sdm_telnet_register(struct sdm_tcp *inter, struct sdm_login *login)
{
	sdm_telnet_write(inter, "\n");
	sdm_telnet_write(inter, SDM_TXT_CHARGEN_NAME_PROMPT);
	sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) telnet_register_read_name, login);
	return(0);
}

/*** Local Functions ***/

static int telnet_register_read_name(struct sdm_login *login, struct sdm_tcp *inter)
{
	int res;
	char buffer[STRING_SIZE];

	if ((res = sdm_telnet_read(inter, buffer, STRING_SIZE - 1)) < 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}
	else if (res == 0)
		return(0);

	if (sdm_user_exists(buffer)) {
		sdm_telnet_write(inter, SDM_TXT_ALREADY_EXISTS);
		sdm_telnet_write(inter, SDM_TXT_CHARGEN_NAME_PROMPT);
	}
	else if (!sdm_user_valid_username(buffer)) {
		sdm_telnet_write(inter, SDM_TXT_INVALID_USERNAME);
		sdm_telnet_write(inter, SDM_TXT_CHARGEN_NAME_PROMPT);
	}
	else if (sdm_login_set_name(login, buffer)) {
		sdm_telnet_write(inter, SDM_TXT_OUT_OF_MEMORY);
		destroy_sdm_interface(SDM_INTERFACE(inter));
	}
	else {
		sdm_telnet_echo(inter, 0);
		sdm_telnet_write(inter, SDM_TXT_CHARGEN_PASSWORD_PROMPT);
		sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) telnet_register_read_password, login);
	}
	return(0);
}

static int telnet_register_read_password(struct sdm_login *login, struct sdm_tcp *inter)
{
	int res;
	char buffer[STRING_SIZE];

	if ((res = sdm_telnet_read(inter, buffer, STRING_SIZE - 1)) < 0) {
		destroy_sdm_interface(SDM_INTERFACE(inter));
		return(-1);
	}
	else if (res == 0)
		return(0);

	sdm_telnet_write(inter, "\n");
	sdm_telnet_encrypt_password(login->name, buffer, STRING_SIZE);
	if (res < 5) {
		sdm_telnet_write(inter, SDM_TXT_INVALID_PASSWORD);
		sdm_telnet_write(inter, SDM_TXT_CHARGEN_PASSWORD_PROMPT);
	}
	else {
		sdm_login_set_password(login, buffer);
		sdm_telnet_write(inter, SDM_TXT_REPEAT_PASSWORD_PROMPT);
		sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) telnet_register_verify_password, login);
	}
	return(0);
}

static int telnet_register_verify_password(struct sdm_login *login, struct sdm_tcp *inter)
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

	sdm_telnet_write(inter, "\n");
	sdm_telnet_encrypt_password(login->name, buffer, STRING_SIZE);
	if (sdm_login_authenticate(login, buffer)) {
		sdm_telnet_echo(inter, 1);
		if (!(user = create_sdm_user(login->name, SDM_INTERFACE(inter)))) {
			sdm_telnet_write(inter, SDM_TXT_OUT_OF_MEMORY);
			destroy_sdm_interface(SDM_INTERFACE(inter));
			return(-1);
		}
		else {
			sdm_login_write_data(login);
			sdm_user_write(user);
			destroy_sdm_object(SDM_OBJECT(login));
			sdm_telnet_write(inter, "\n");
			// TODO invoke the game character generation object scripty stuff or else put that
			// invokation in telnet_run() such that if something happens, and the user logs back
			// in without finishing the character generation, it will continue or start over.
			sdm_telnet_run(inter, user);
		}
	}
	else  {
		sdm_telnet_write(inter, SDM_TXT_PASSWORDS_DONT_MATCH);
		sdm_telnet_write(inter, SDM_TXT_CHARGEN_PASSWORD_PROMPT);
		sdm_interface_set_callback(SDM_INTERFACE(inter), IO_COND_READ, (callback_t) telnet_register_read_password, login);
	}
	return(0);
}


