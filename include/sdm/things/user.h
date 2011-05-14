/*
 * Object Name:	user.h
 * Description:	User Object
 */

#ifndef _SDM_THINGS_USER_H
#define _SDM_THINGS_USER_H

#include <string>
#include <stdio.h>
#include <stdarg.h>

#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

#define MOO_UBF_GUEST		0x0001		/// User is a guest (Don't save)

class MooUser : public MooThing {
	int m_bits;
	MooTask *m_task;

    public:
	MooUser(const char *name, int bits = 0, moo_id_t id = -1, moo_id_t parent = -1);
	virtual ~MooUser();
	static MooUser *make_guest(const char *name);

	virtual int load();
	virtual int save();

	MooThing *get_thing() { return(this); }

	int connect(MooTask *task);
	void disconnect();

	int notify(int type, MooThing *thing, MooThing *channel, const char *text);

	/// Authentication Methods
	static int logged_in(const char *name);
	static int valid_username(const char *name);
	static MooUser *login(const char *name, const char *passwd);
	static int exists(const char *name);
	static void encrypt_password(const char *salt, char *passwd, int max);
};

extern MooObjectType moo_user_obj_type;

int init_user(void);
void release_user(void);
MooObject *make_moo_user(MooDataFile *data);

#endif

