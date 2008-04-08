/*
 * Object Name:	user.c
 * Description:	User Object
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/things/utils.h>
#include <sdm/interfaces/interface.h>
#include <sdm/processors/form.h>
#include <sdm/processors/processor.h>
#include <sdm/processors/interpreter.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/user.h>

struct sdm_object_type sdm_user_obj_type = {
	&sdm_thing_obj_type,
	"user",
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
	const char *str;
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	if (user_list)
		return(1);
	if (!(user_list = create_sdm_hash(0, -1, NULL)))
		return(-1);

	/** Load all the users into memory in a disconnected state */
	if (!(data = sdm_data_open("etc/passwd.xml", SDM_DATA_READ, "passwd")))
		return(-1);
	do {
		if ((str = sdm_data_read_name(data)) && !strcmp(str, "user")) {
			sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
			// TODO make sure the user is in it's proper disconnected state (in the cryolocker)
			create_sdm_user(buffer);
		}
	} while (sdm_data_read_next(data));
	sdm_data_close(data);
	return(0);
}

void release_user(void)
{
	struct sdm_user *user;

	if (user_list) {
		sdm_hash_traverse_reset(user_list);
		while ((user = sdm_hash_traverse_next(user_list))) {
			if (user->inter)
				destroy_sdm_object(SDM_OBJECT(user->inter));
			destroy_sdm_object(SDM_OBJECT(user));
		}
		destroy_sdm_hash(user_list);
	}
	user_list = NULL;
}

struct sdm_user *create_sdm_user(const char *name)
{
	struct sdm_user *user;

	if (!(user = sdm_hash_find(user_list, name))
	    && !(user = (struct sdm_user *) create_sdm_object(&sdm_user_obj_type, 3, SDM_USER_ARGS(name, SDM_NO_ID, 0))))
		return(NULL);
	return(user);
}

int sdm_user_init(struct sdm_user *user, int nargs, va_list va)
{
	const char *name;

	if (nargs > 1) {
		name = va_arg(va, const char *);
		if (!name || !sdm_user_valid_username(name) || !(user->name = make_string("%s", name)))
			return(-1);
		nargs--;
	}

	/** If there is already a user with that name then fail */
	if (sdm_hash_add(user_list, name, user))
		return(-1);
	if (sdm_thing_init(SDM_THING(user), nargs, va) < 0)
		return(-1);
	sdm_user_read(user);
	return(0);
}

void sdm_user_release(struct sdm_user *user)
{
	/** Save the user information to the user's file, and disconnect */
	sdm_user_disconnect(user);

	/** Release the user's other resources */
	sdm_hash_remove(user_list, user->name);
	memory_free(user->name);
	destroy_sdm_object(SDM_OBJECT(user->inter));

	/** Release the superclass */
	sdm_thing_release(SDM_THING(user));
}

int sdm_user_connect(struct sdm_user *user, struct sdm_interface *inter)
{
	double room;
	struct sdm_thing *location;

	if (user->inter)
		destroy_sdm_object(SDM_OBJECT(user->inter));
	user->inter = inter;

	if (SDM_THING(user)->parent > 0)
		user->proc = SDM_PROCESSOR(create_sdm_object(SDM_OBJECT_TYPE(&sdm_interpreter_obj_type), 0));
	else
		user->proc = SDM_PROCESSOR(create_sdm_object(SDM_OBJECT_TYPE(&sdm_form_obj_type), 1, "etc/register.xml"));

	/** Move the user to the last location recorded or to a safe place if there is no last location */
	if (((room = sdm_get_number_property(SDM_THING(user), "last_location")) > 0)
	    && (location = sdm_thing_lookup_id(room)))
		sdm_moveto(SDM_THING(user), SDM_THING(user), location, NULL);
	else
		// TODO you should do this some othe way
		sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_thing_lookup_id(50), NULL);
		//sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_interpreter_find_object(NULL, "/lost+found"), NULL);

	if (!user->proc)
		return(-1);
	sdm_processor_startup(user->proc, user);
	return(0);
}

void sdm_user_disconnect(struct sdm_user *user)
{
	struct sdm_number *number;

	/** Shutdown the input processor */
	if (user->proc) {
		sdm_processor_shutdown(user->proc, user);
		destroy_sdm_object(SDM_OBJECT(user->proc));
		user->proc = NULL;
	}

	if ((number = SDM_NUMBER(sdm_thing_get_property(SDM_THING(user), "last_location", &sdm_number_obj_type)))
	    || ((number = create_sdm_number(-1)) && !sdm_thing_set_property(SDM_THING(user), "last_location", SDM_OBJECT(number)))) {
		number->num = SDM_THING(user)->location ? SDM_THING(user)->location->id : -1;
	}
	// TODO how do you tell this function to forcefully remove the user
	if (SDM_THING(user)->location)
		sdm_thing_remove(SDM_THING(user)->location, SDM_THING(user));

	/** Save the user information to the user's file only if we were already connected */
	if (user->inter)
		sdm_user_write(user);
	user->inter = NULL;
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
	struct sdm_user *user;

	if ((user = sdm_hash_find(user_list, name)) && user->inter)
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

