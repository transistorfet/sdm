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
	std::string *m_name;

    public:
	MooUser(const char *name, int bits = 0, moo_id_t id = -1, moo_id_t parent = 0);
	virtual ~MooUser();
	static MooUser *make_guest(const char *name);
	int convert_guest();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int load();
	int save();
	int connect(MooTask *task);
	void disconnect();

	int notify(int type, MooThing *thing, MooThing *channel, const char *text);

	/// Authentication Methods
	static int exists(const char *name);
	static int logged_in(const char *name);
	static int valid_username(const char *name);
	static MooUser *login(const char *name, const char *passwd);
	static void encrypt_password(const char *salt, char *passwd, int max);
	static MooUser *get(const char *name);

    public:
	/// Accessors
	const char *name() { return(m_name->c_str()); }
};

extern MooObjectType moo_user_obj_type;

int init_user(void);
void release_user(void);
MooObject *moo_user_create(void);

#endif

