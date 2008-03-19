/*
 * Object Name:	user.h
 * Description:	User Object
 */

#ifndef _SDM_OBJS_USER_H
#define _SDM_OBJS_USER_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/string.h>
#include <sdm/objs/processor.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/objs/mobile.h>

#define SDM_USER(ptr)		( (struct sdm_user *) (ptr) )

struct sdm_user {
	struct sdm_mobile mobile;
	struct sdm_interface *inter;
	struct sdm_processor *proc;
	string_t name;

};

#define SDM_USER_ARGS(name, inter, id, parent)		(name), (inter), SDM_MOBILE_ARGS((id), (parent))

extern struct sdm_object_type sdm_user_obj_type;

int init_user(void);
int release_user(void);

struct sdm_user *create_sdm_user(const char *, struct sdm_interface *);

int sdm_user_init(struct sdm_user *, va_list);
void sdm_user_release(struct sdm_user *);
int sdm_user_read_entry(struct sdm_user *, const char *, struct sdm_data_file *);
int sdm_user_write_data(struct sdm_user *, struct sdm_data_file *);

int sdm_user_exists(const char *);
int sdm_user_logged_in(const char *);
int sdm_user_valid_username(const char *);

static inline int sdm_user_read(struct sdm_user *user) {
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", user->name);
	return(sdm_object_read_file(SDM_OBJECT(user), buffer, "user"));
}

static inline int sdm_user_write(struct sdm_user *user) {
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", user->name);
	return(sdm_object_write_file(SDM_OBJECT(user), buffer, "user"));
}

#endif

