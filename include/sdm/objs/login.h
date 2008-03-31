/*
 * Object Name:	login.h
 * Description:	Login Object
 */

#ifndef _SDM_OBJS_LOGIN_H
#define _SDM_OBJS_LOGIN_H

#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>

#define SDM_LOGIN(ptr)		( (struct sdm_login *) (ptr) )

struct sdm_login {
	struct sdm_object object;
	string_t name;
	// TODO assume just password authentication for now
	string_t password;
};

extern struct sdm_object_type sdm_login_obj_type;

#define create_sdm_login()	\
	( (struct sdm_login *) create_sdm_object(&sdm_login_obj_type, 0) )

int sdm_login_init(struct sdm_login *, int, va_list);
void sdm_login_release(struct sdm_login *);

int sdm_login_write_data(struct sdm_login *);

int sdm_login_set_name(struct sdm_login *, const char *);
int sdm_login_set_password(struct sdm_login *, const char *);
int sdm_login_authenticate(struct sdm_login *, const char *);

#endif


