/*
 * Object Name:	user.cpp
 * Description:	User Object
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/tasks/task.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/things/thing.h>
#include <sdm/things/user.h>

#include <sdm/code/code.h>

#define USER_INIT_SIZE		64

MooObjectType moo_user_obj_type = {
	&moo_thing_obj_type,
	"user",
	typeid(MooUser).name(),
	(moo_type_make_t) make_moo_user
};

static MooHash<MooUser *> *user_list = NULL;

int init_user(void)
{
	moo_id_t id;
	const char *str;
	char buffer[STRING_SIZE];
	MooDataFile *data;
	MooUser *user;

	if (user_list)
		return(1);
	user_list = new MooHash<MooUser *>(USER_INIT_SIZE, MOO_HBF_REMOVE | MOO_HBF_DELETEALL);
	moo_object_register_type(&moo_user_obj_type);

	/// Load all the users into memory in a disconnected state
	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib_string("name", buffer, STRING_SIZE);
			if (data->read_children()) {
				while (data->read_next()) {
					str = data->read_name();
					if (!strcmp(str, "thing"))
						id = data->read_integer_entry();
				}
				data->read_parent();
				user = new MooUser(buffer, 0, id);
				// TODO should this be fixed?
				//user->cryolocker_store();
			}
		}
	} while (data->read_next());
	delete data;
	return(0);
}

void release_user(void)
{
	if (!user_list)
		return;
	delete user_list;
	user_list = NULL;
}


MooObject *make_moo_user(MooDataFile *data)
{
	int id;
	MooThing *thing;

	id = data->read_integer();
	// TODO is this right to return the nil value, or should you create a new nil value
	if (!(thing = MooThing::lookup(id)))
		return(make_moo_nil(NULL));
	return(dynamic_cast<MooUser *>(thing));
}

MooUser::MooUser(const char *name, int bits, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_bits = bits;
	m_task = NULL;

	if (!name || !MooUser::valid_username(name))
		throw MooException("User name error");

	this->load();
	if (user_list->set(name, this))
		throw MooException("Unable to add user to list: %s", name);
}

MooUser::~MooUser()
{
	/// Save the user information to the user's file, and disconnect
	this->disconnect();

	///If we are associated with a task, then notify it that we are dying
	if (m_task)
		m_task->purge(this);
}

MooUser *MooUser::make_guest(const char *name)
{
	MooUser *user;
	MooThing *parent;

	if (MooUser::exists(name))
		throw MooException("User already exists.  Cannot make guest.");
	// TODO should these references perhaps be stored in a hash table of some kind which is searched with $thing references
	if (!(parent = MooThing::reference(MOO_GENERIC_USER)))
		throw moo_thing_not_found;
	user = new MooUser(name, MOO_UBF_GUEST, MOO_NEW_ID, parent->id());
	user->resolve_property("name", new MooString(name));
	user->owner(user->m_id);
	user->resolve_property("description", new MooString("You see a new person who looks rather out-of-place."));
	// TODO is this the correct way to moving a user to the starting location?
	//user->moveto(user, NULL, MooThing::reference(MOO_START_ROOM));
	return(user);
}

int MooUser::load()
{
	if (m_bits & MOO_UBF_GUEST)
		return(-1);
	return(MooThing::load());
}

int MooUser::save()
{
	if (m_bits & MOO_UBF_GUEST)
		return(-1);
	return(MooThing::save());
}

int MooUser::connect(MooTask *task)
{
	if (m_task)
		return(-1);
	/// If an error occurs and we return early, m_task will not be set, so if disconnect() is called, we wont save the user
	m_task = task;
	return(0);
}

void MooUser::disconnect()
{
	/// If no task is set, then we aren't connected, and therefore should not disconnect
	if (!m_task)
		return;

	try {
		/// Save the user information to the user's file only if we were already connected
		this->save();
		m_task->purge(this);
		m_task = NULL;

		if (m_bits & MOO_UBF_GUEST) {
			// TODO if user is a guest, delete the 'thing'
		}
	}
	catch (...) {
		moo_status("USER: Error saving user data, %d", m_id);
	}
}

int MooUser::notify(int type, MooThing *thing, MooThing *channel, const char *text)
{
	if (!m_task)
		return(-1);
	// TODO permissions check!? I guess on task
	return(m_task->notify(type, thing, channel, text));
}


int MooUser::logged_in(const char *name)
{
	MooUser *user;

	if ((user = user_list->get(name)) && user->m_task)
		return(1);
	return(0);
}

int MooUser::valid_username(const char *name)
{
	int i;

	for (i = 0; name[i] != '\0'; i++) {
		if (!(((name[i] >= 0x30) && (name[i] <= 0x39))
		    || ((name[i] >= 0x41) && (name[i] <= 0x5a))
		    || ((name[i] >= 0x61) && (name[i] <= 0x7a))
		    || (name[i] == '-') || (name[i] == '.') || (name[i] == '_')))
			return(0);
	}
	return(1);
}

MooUser *MooUser::login(const char *name, const char *passwd)
{
	moo_id_t id;
	const char *str;
	MooDataFile *data;
	char buffer[STRING_SIZE];

	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib_string("name", buffer, STRING_SIZE);
			if (strcmp(buffer, name))
				continue;
			if (data->read_children()) {
				do {
					str = data->read_name();
					if (!strcmp(str, "thing"))
						id = data->read_integer_entry();
					else if (!strcmp(str, "password")) {
						data->read_string_entry(buffer, STRING_SIZE);
						if (!strcmp(buffer, passwd))
							passwd = NULL;
						else {
							delete data;
							return(NULL);
						}
					}
				} while (data->read_next());
				if (passwd)
					break;
				memset(buffer, '\0', STRING_SIZE);
				return(dynamic_cast<MooUser *>(MooThing::lookup(id)));
			}
			break;
		}
	} while (data->read_next());
	delete data;
	return(NULL);
}

int MooUser::exists(const char *name)
{
	const char *str;
	MooDataFile *data;
	char buffer[STRING_SIZE];

	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib_string("name", buffer, STRING_SIZE);
			if (!strcmp(buffer, name)) {
				delete data;
				return(1);
			}
		}
	} while (data->read_next());
	delete data;
	return(0);
}


void MooUser::encrypt_password(const char *salt, char *passwd, int max)
{
	char *enc;

	enc = crypt(passwd, salt);
	strncpy(passwd, enc, max);
}


