/*
 * Object Name:	user.c
 * Description:	User Object
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/hash.h>
#include <sdm/string.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/form.h>
#include <sdm/objs/string.h>
#include <sdm/objs/processor.h>
#include <sdm/objs/interpreter.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/container.h>
#include <sdm/objs/mobile.h>
#include <sdm/objs/user.h>

struct sdm_object_type sdm_user_obj_type = {
	&sdm_mobile_obj_type,
	sizeof(struct sdm_user),
	NULL,
	(sdm_object_init_t) sdm_user_init,
	(sdm_object_release_t) sdm_user_release,
	(sdm_object_read_entry_t) sdm_user_read_entry,
	(sdm_object_write_data_t) sdm_user_write_data
};

static struct sdm_hash *user_list = NULL;

int init_user(void)
{
	if (user_list)
		return(1);
	if (!(user_list = create_sdm_hash(0, -1, NULL)))
		return(-1);
	return(0);
}

void release_user(void)
{
	struct sdm_user *user;

	if (user_list) {
		sdm_hash_traverse_reset(user_list);
		while ((user = sdm_hash_traverse_next(user_list)))
			destroy_sdm_object(SDM_OBJECT(user));
		destroy_sdm_hash(user_list);
	}
	user_list = NULL;
}

struct sdm_user *create_sdm_user(const char *name, struct sdm_interface *inter)
{
	struct sdm_user *user;

	if ((user = sdm_hash_find(user_list, name)))
		return(user);
	return((struct sdm_user *) create_sdm_object(&sdm_user_obj_type, SDM_USER_ARGS(name, inter, SDM_NO_ID, 0)));
}

int sdm_user_init(struct sdm_user *user, va_list va)
{
	const char *name;

	name = va_arg(va, const char *);
	if (!name || !sdm_user_valid_username(name) || !(user->name = create_string("%s", name)))
		return(-1);
	user->inter = va_arg(va, struct sdm_interface *);

	/** If there is already a user with that name then fail */
	if (sdm_hash_add(user_list, name, user))
		return(-1);

	if (sdm_mobile_init(SDM_MOBILE(user), va) < 0)
		return(-1);
	if (sdm_user_exists(user->name)) {
		sdm_user_read(user);
		// TODO should this be specified and loaded from the file?
		if (!(user->proc = SDM_PROCESSOR(create_sdm_object(SDM_OBJECT_TYPE(&sdm_interpreter_obj_type)))))
			return(-1);
	}
	else {
		// TODO should there be another way to find out what form to use to register users?
		if (!(user->proc = SDM_PROCESSOR(create_sdm_object(SDM_OBJECT_TYPE(&sdm_form_obj_type), "etc/register.xml"))))
			return(-1);
		// TODO should you write the user to disk at this point?
	}
	return(0);
}

void sdm_user_release(struct sdm_user *user)
{
	/** Save the user information to the user's file */
	sdm_user_write(user);

	/** Shutdown the input processor */
	// TODO we shouldn't shut down the processor here since it's shutdown in telnet.  If we are
	//	destroying the user without destroying the connection to initiate it, we are doing it wrong
	//	(possibly the server is dying with users logged in, thus we don't need to shutdown).
	//sdm_processor_shutdown(user->proc, user);
	destroy_sdm_object(SDM_OBJECT(user->proc));

	/** Release the user's other resources */
	sdm_hash_remove(user_list, user->name);
	destroy_string(user->name);
	destroy_sdm_object(SDM_OBJECT(user->inter));

	/** Release the superclass */
	sdm_mobile_release(SDM_MOBILE(user));
}

int sdm_user_read_entry(struct sdm_user *user, const char *type, struct sdm_data_file *data)
{
	if (!strcmp(type, "name")) {
		// TODO this should already have been set but maybe we can generate an error if it doesn't match
	}
	// TODO read processor type and settings
	// TODO read telnet/interface settings if we are going to do that
	else
		return(SDM_NOT_HANDLED);
	return(SDM_HANDLED);
}

int sdm_user_write_data(struct sdm_user *user, struct sdm_data_file *data)
{
	sdm_data_write_string_entry(data, "name", user->name);
	sdm_data_write_integer_entry(data, "lastseen", time(NULL));
	// TODO write telnet/interface settings
	// TODO write processor settings and types
	return(0);
}


int sdm_user_exists(const char *name)
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", name);
	return(sdm_data_file_exists(buffer));
}

int sdm_user_logged_in(const char *name)
{
	if (sdm_hash_find(user_list, name))
		return(1);
	return(0);
}

int sdm_user_valid_username(const char *name)
{
	int i;

	for (i = 0; name[i] != '\0'; i++) {
		if (!(((name[i] >= 0x30) && (name[i] <= 0x39))
		    || ((name[i] >= 0x41) && (name[i] <= 0x5a))
		    || ((name[i] >= 0x61) && (name[i] <= 0x7a))
		    || (name[i] == '-') || (name[i] == '.') || (name[i] == '_')))
			return(0);
	}
	return(1);
}

