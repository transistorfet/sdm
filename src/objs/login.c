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
	const char *str;
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	if (!login->name)
		return(-1);
	if (!(data = sdm_data_open("etc/passwd.xml", SDM_DATA_READ, "passwd")))
		return(-1);
	do {
		if ((str = sdm_data_read_name(data)) && !strcmp(str, "user")) {
			sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
			if (!strcmp(buffer, login->name)) {
				sdm_data_read_children(data);
				do {
					if ((str = sdm_data_read_name(data)) && !strcmp(str, "password")) {
						sdm_data_read_string(data, buffer, STRING_SIZE);
						login->password = create_string("%s", buffer);
					}
				} while (sdm_data_read_next(data));
				sdm_data_read_parent(data);
			}
		}
	} while (sdm_data_read_next(data));
	sdm_data_close(data);
	return(0);
}

int sdm_login_write_data(struct sdm_login *login)
{
	const char *str;
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	if (!(data = sdm_data_open("etc/passwd.xml", SDM_DATA_READ_WRITE, "passwd")))
		return(-1);
	/** Write the new entry at the top (since newer entries are more likely to be accessed) */
	sdm_data_write_begin_entry(data, "user");
	sdm_data_write_attrib(data, "name", login->name);
	sdm_data_write_string_entry(data, "password", login->password);
	sdm_data_write_end_entry(data);
	do {
		if ((str = sdm_data_read_name(data)) && !strcmp(str, "user")) {
			sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
			if (strcmp(buffer, login->name)) {
				/** Write all entries as they appear except the one we are replacing */
				sdm_data_write_current(data);
			}
		}
	} while (sdm_data_read_next(data));
	sdm_data_close(data);
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


