/*
 * Object Name:	login.c
 * Description:	Login Object
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/hash.h>
#include <sdm/string.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/login.h>

struct sdm_object_type sdm_login_obj_type = {
	sizeof(struct sdm_login),
	NULL,
	(sdm_object_init_t) sdm_login_init,
	(sdm_object_release_t) sdm_login_release
};

int sdm_login_init(struct sdm_login *login, va_list va)
{
	return(0);
}

void sdm_login_release(struct sdm_login *login)
{
	if (login->name)
		destroy_string(login->name);
}


int sdm_login_read_data(struct sdm_login *login)
{
	if (!login->name)
		return(-1);
	// TODO read the data from the file and find the information for the login name stored in this
	//	login object
	return(0);
}

int sdm_login_write_data(struct sdm_login *login)
{
	// TODO write the recorded information for login to the password database
	return(0);
}


int sdm_login_set_name(struct sdm_login *login, const char *name)
{
	if (login->name)
		destroy_string(login->name);
	if (!(login->name = create_string("%s", name)))
		return(-1);
	return(0);
}

int sdm_login_set_password(struct sdm_login *login, const char *password)
{
	if (login->password)
		destroy_string(login->password);
	if (!(login->password = create_string("%s", password)))
		return(-1);
	return(0);
}

int sdm_login_authenticate(struct sdm_login *login, const char *password)
{
// TODO hack, remove after testing
return(1);
	if (!login->password) {
		/** Read in the authentication data for this login session */
		sdm_login_read_data(login);
		if (!login->password)
			return(0);
	}
	if (!strcmp(login->password, password))
		return(1);
	return(0);
}


