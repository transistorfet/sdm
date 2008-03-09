/*
 * Object Name:	user.h
 * Description:	User Object
 */

#ifndef _SDM_OBJS_USER_H
#define _SDM_OBJS_USER_H

#include <stdarg.h>

#include <sdm/string.h>
#include <sdm/interface/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/mobile.h>

#define SDM_USER(ptr)		( (struct sdm_user *) (ptr) )

struct sdm_user {
	struct sdm_mobile mobile;
	struct sdm_interface *inter;
	string_t name;
	string_t fullname;
	string_t password;

};

extern struct sdm_object_type sdm_user_obj_type;

int init_user(void);
int release_user(void);

struct sdm_user *create_sdm_user(const char *, struct sdm_interface *);

int sdm_user_init(struct sdm_user *, va_list);
void sdm_user_release(struct sdm_user *);

int sdm_user_exists(const char *);
int sdm_user_logged_in(const char *);

#endif

