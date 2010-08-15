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
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

class MooUser : public MooThing {
	MooTask *m_task;
	std::string *m_name;
    public:
	MooUser(const char *name, moo_id_t id = -1, moo_id_t parent = 0);
	virtual ~MooUser();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int load();
	int save();
	int connect(MooTask *task);
	void disconnect();

	int command(const char *text);
	int command(const char *action, const char *text);
	int command(const char *action, MooThing *object, MooThing *target);
	int command(const char *action, MooArgs *args);
	MooThing *find_thing(const char *name);
	int print(MooThing *channel, MooThing *thing, const char *text);
	int printf(MooThing *channel, MooThing *thing, const char *fmt, ...);
	int print(MooThing *channel, MooThing *thing, MooArgs *args, const char *text);
	int printf(MooThing *channel, MooThing *thing, MooArgs *args, const char *fmt, ...);

	static int exists(const char *name);
	static int logged_in(const char *name);
	static int valid_username(const char *name);
	static MooUser *register_new(const char *name, ...);
	static MooUser *login(const char *name, const char *passwd);
	static void encrypt_password(const char *salt, char *passwd, int max);
};

extern MooObjectType moo_user_obj_type;

int init_user(void);
void release_user(void);
MooObject *moo_user_create(void);

#endif

