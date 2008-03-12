/*
 * Object Name:	user.c
 * Description:	User Object
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/hash.h>
#include <sdm/string.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/processor.h>
#include <sdm/objs/interpreter.h>
#include <sdm/interface/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>
#include <sdm/objs/container.h>
#include <sdm/objs/mobile.h>
#include <sdm/objs/user.h>

struct sdm_object_type sdm_user_obj_type = {
	&sdm_mobile_obj_type,
	sizeof(struct sdm_user),
	NULL,
	(sdm_object_init_t) sdm_user_init,
	(sdm_object_release_t) sdm_user_release
};

static struct sdm_hash *user_list = NULL;

int init_user(void)
{
	if (user_list)
		return(1);
	if (!(user_list = create_sdm_hash(0, NULL)))
		return(-1);
	return(0);
}

int release_user(void)
{
	struct sdm_user *user;

	if (user_list) {
		sdm_hash_traverse_reset(user_list);
		while ((user = sdm_hash_traverse_next(user_list)))
			destroy_sdm_object(SDM_OBJECT(user));
		destroy_sdm_hash(user_list);
	}
	user_list = NULL;
	return(0);
}

struct sdm_user *create_sdm_user(const char *name, struct sdm_interface *inter)
{
	struct sdm_user *user;

	if ((user = sdm_hash_find(user_list, name)))
		return(user);
	return((struct sdm_user *) create_sdm_object(&sdm_user_obj_type, name, inter));
}

int sdm_user_init(struct sdm_user *user, va_list va)
{
	const char *name;

	name = va_arg(va, const char *);
	if (!name || (*name == '\0') || !(user->name = create_string("%s", name)))
		return(-1);
	user->inter = va_arg(va, struct sdm_interface *);

	/** If there is already a user with that name then fail */
	if (sdm_hash_add(user_list, name, user))
		return(-1);

	if (CALL_SDM_OBJECT_INIT((sdm_object_init_t) sdm_mobile_init, SDM_OBJECT(user)) < 0)
		return(-1);
	sdm_user_read_data(user);

	// TODO this should probably be created in read_data or as a result of reading data
	if (!(user->proc = (struct sdm_processor *) create_sdm_object(&sdm_interpreter_obj_type)))
		return(-1);

	// TODO this is temporary until you get a way to determine a user's location based on the datafile
	sdm_container_add(SDM_CONTAINER(sdm_world_get_root()), SDM_ACTIONABLE(user));
	return(0);
}

void sdm_user_release(struct sdm_user *user)
{
	/** Save the user information to the user's file */
	sdm_user_write_data(user);
	/** Shutdown the input processor */
	sdm_processor_shutdown(user->proc, user);
	destroy_sdm_object(SDM_OBJECT(user->proc));
	/** Release the user's other resources */
	sdm_hash_remove(user_list, user->name);
	destroy_string(user->name);
	destroy_sdm_interface(user->inter);
	// TODO do all the other releasing
	/** Release the superclass */
	sdm_mobile_release(SDM_MOBILE(user));
}


int sdm_user_read_data(struct sdm_user *user)
{
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	snprintf(buffer, STRING_SIZE, "users/%s.xml", user->name);
	if (!(sdm_data_file_exists(buffer)) || !(data = sdm_data_open(buffer, SDM_DATA_READ, "user")))
		return(-1);
	// TODO read in data
	sdm_data_close(data);
	return(0);
}

int sdm_user_write_data(struct sdm_user *user)
{
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	snprintf(buffer, STRING_SIZE, "users/%s.xml", user->name);
	if (!(data = sdm_data_open(buffer, SDM_DATA_WRITE, "user")))
		return(-1);
	sdm_data_write_string_entry(data, "name", user->name);
	// TODO write everything else
	sdm_data_close(data);
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


int sdm_user_startup(struct sdm_user *user)
{
	// TODO should telnet or whatever interface call this directly?
	sdm_processor_startup(user->proc, user);
	return(0);
}

int sdm_user_process_input(struct sdm_user *user, char *input)
{
	// TODO should telnet or whatever interface call this directly?
	return(sdm_processor_process(user->proc, user, input));
}


// TODO should these go here?
int sdm_user_tell(struct sdm_user *user, const char *fmt, ...)
{
	va_list va;
	char buffer[STRING_SIZE];

	va_start(va, fmt);
	vsnprintf(buffer, STRING_SIZE, fmt, va);
	va_end(va);
	return(SDM_INTERFACE_WRITE(user->inter, buffer));
}

int sdm_user_announce(struct sdm_user *user, const char *fmt, ...)
{
	va_list va;
	char buffer[STRING_SIZE];
	struct sdm_actionable *cur;

	va_start(va, fmt);
	vsnprintf(buffer, STRING_SIZE, fmt, va);
	va_end(va);
	if (!SDM_ACTIONABLE(user)->owner)
		return(0);
	for (cur = SDM_ACTIONABLE(user)->owner->objects; cur; cur = cur->next) {
		if ((SDM_OBJECT(cur)->type == &sdm_user_obj_type) && (SDM_USER(cur) != user))
			SDM_INTERFACE_WRITE(SDM_USER(cur)->inter, buffer);
	}
	return(0);
}

