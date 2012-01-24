/*
 * Object Name:	users.h
 * Description:	User Methods
 */

#ifndef _SDM_LIB_USERS_H
#define _SDM_LIB_USERS_H

#include <string>
#include <stdio.h>
#include <stdarg.h>

class MooThing;

MooThing *user_make_guest(const char *name);
int user_logged_in(const char *name);
int user_valid_username(const char *name);
MooThing *user_login(const char *name, const char *passwd);
int user_exists(const char *name);
void user_encrypt_password(const char *salt, char *passwd, int max);

#endif

