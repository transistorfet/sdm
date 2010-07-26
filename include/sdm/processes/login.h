/*
 * Object Name:	login.h
 * Description:	Login Process
 */

#ifndef _SDM_PROCESS_LOGIN_H
#define _SDM_PROCESS_LOGIN_H

#include <string>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/processes/process.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>

// TODO this would be a specific process for handling telnet connections (IRC would use a different such process)

#define SDM_LOGIN(ptr)		( (struct sdm_login *) (ptr) )

class MooLogin : public MooProcess {
	std::string name;
	// TODO assume just password authentication for now
	std::string password;
    public:
	MooLogin();
	~MooLogin();

	int initialize(MooUser *user);
	int idle(MooUser *user, char *buffer);
	int handle(MooInterface *inter);
	int release(MooUser *user);
};

extern MooObjectType moo_login_obj_type;

MooObject *moo_login_create(void);

int sdm_login_write_data(struct sdm_login *);

int sdm_login_set_name(struct sdm_login *, const char *);
int sdm_login_set_password(struct sdm_login *, const char *);
int sdm_login_authenticate(struct sdm_login *, const char *);

#endif


