/*
 * Object Name:	user.h
 * Description:	User Object
 */

#ifndef _SDM_THINGS_USER_H
#define _SDM_THINGS_USER_H

#include <string>
#include <stdio.h>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/interfaces/interface.h>
#include <sdm/processes/process.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

class MooUser : public MooThing {
	MooInterface *m_inter;
	MooProcess *m_proc;
	string_t m_name;
    public:
	MooUser(const char *name, moo_id_t id = -1, moo_id_t parent = 0);
	virtual ~MooUser();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int connect(MooInterface *inter);
	void disconnect();

	int read_file();
	int write_file();
};

extern MooObjectType moo_user_obj_type;

int init_user(void);
void release_user(void);
MooObject *moo_user_create(void);

int moo_user_exists(const char *);
int moo_user_logged_in(const char *);
int moo_user_valid_username(const char *);

#endif

