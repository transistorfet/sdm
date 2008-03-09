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
#include <sdm/interface/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/mobile.h>
#include <sdm/objs/user.h>

struct sdm_object_type sdm_user_obj_type = {
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
	char buffer[STRING_SIZE];
	struct sdm_data_file *data;

	name = va_arg(va, const char *);
	if (!(user->name = create_string("%s", name)))
		return(-1);
	user->inter = va_arg(va, struct sdm_interface *);

	/** If there is already a user with that name then fail */
	if (sdm_hash_add(user_list, name, user))
		return(-1);

	CALL_SDM_OBJECT_INIT((sdm_object_init_t) sdm_mobile_init, SDM_OBJECT(user));

	snprintf(buffer, STRING_SIZE, "users/%s.xml", name);
	if (!(sdm_data_file_exists(buffer)) || !(data = sdm_data_open(buffer, "user")))
		return(-1);
	// TODO initialize user

	return(0);
}

void sdm_user_release(struct sdm_user *user)
{
	// TODO do all the other releasing
	sdm_hash_remove(user_list, user->name);
	destroy_string(user->name);
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


