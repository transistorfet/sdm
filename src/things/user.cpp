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
#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/tasks/task.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/interfaces/interface.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/user.h>

#define USER_INIT_SIZE		64

MooObjectType moo_user_obj_type = {
	&moo_thing_obj_type,
	"user",
	typeid(MooUser).name(),
	(moo_type_create_t) NULL
};

static MooHash<MooUser *> *user_list = NULL;

int init_user(void)
{
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
			user = new MooUser(buffer);
			user->cryolocker_store();
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


MooUser::MooUser(const char *name, int bits, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_bits = bits;
	m_task = NULL;
	m_name = NULL;

	if (!name || !MooUser::valid_username(name))
		throw MooException("User name error");
	m_name = new std::string(name);

	if (user_list->set(name, this))
		throw MooException("Unable to add user to list: %s", name);
	this->load();
}

MooUser::~MooUser()
{
	/// Save the user information to the user's file, and disconnect
	this->disconnect();

	///If we are associated with a task, then notify it that we are dying
	if (m_task)
		m_task->purge(this);

	/// Release the user's other resources
	if (m_name) {
		user_list->remove(m_name->c_str());
		delete m_name;
	}
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
	user->set_property("name", name);
	user->init();
	user->owner(user->m_id);
	user->set_property("description", "You see a new person who looks rather out-of-place.");
	// TODO is this the correct way to moving a user to the starting location?
	user->moveto(MooThing::reference(MOO_START_ROOM), NULL);
	return(user);
}

int MooUser::convert_guest()
{
	// TODO Convert a guest user into full user??
	return(0);
}

int MooUser::load()
{
	char buffer[STRING_SIZE];

	if (m_bits & MOO_UBF_GUEST)
		return(-1);
	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name->c_str());
	return(MooObject::read_file(buffer, "user"));
}

int MooUser::save()
{
	char buffer[STRING_SIZE];

	if (m_bits & MOO_UBF_GUEST)
		return(-1);
	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name->c_str());
	return(MooObject::write_file(buffer, "user"));
}

int MooUser::connect(MooTask *task)
{
	if (m_task)
		return(-1);
	this->cryolocker_revive();

	/// If an error occurs and we return early, m_task will not be set, so if disconnect() is called, we wont save the user
	m_task = task;
	return(0);
}

void MooUser::disconnect()
{
	this->cryolocker_store();

	/// Save the user information to the user's file only if we were already connected
	if (m_task) {
		this->save();
		m_task->purge(this);
	}
	m_task = NULL;
}


int MooUser::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "name")) {
		// TODO this should already have been set but maybe we can generate an error if it doesn't match
	}
	else
		return(MooThing::read_entry(type, data));
	return(0);
}

int MooUser::write_data(MooDataFile *data)
{
	MooThing::write_data(data);
	data->write_string_entry("name", m_name->c_str());
	data->write_integer_entry("lastseen", time(NULL));
	return(0);
}


int MooUser::notify(int type, MooThing *channel, MooThing *thing, const char *text)
{
	if (!m_task)
		return(-1);
	// TODO permissions check!? I guess on task
	return(m_task->notify(type, channel, thing, text));
}


int MooUser::exists(const char *name)
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", name);
	return(moo_data_file_exists(buffer));
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
	MooUser *user;
	MooDataFile *data;
	const char *str;
	char buffer[STRING_SIZE];

	// TODO overwrite buffers used to store password as a security measure
	if (!(user = user_list->get(name)) || user->m_task)
		return(NULL);
	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib_string("name", buffer, STRING_SIZE);
			if (strcmp(buffer, name))
				continue;
			data->read_children();
			do {
				if ((str = data->read_name()) && !strcmp(str, "password")) {
					data->read_string_entry(buffer, STRING_SIZE);
					if (!strcmp(buffer, passwd)) {
						delete data;
						return(user);
					}
					return(NULL);
				}
			} while (data->read_next());
			break;
		}
	} while (data->read_next());
	delete data;
	return(NULL);
}

void MooUser::encrypt_password(const char *salt, char *passwd, int max)
{
	char *enc;

	enc = crypt(passwd, salt);
	strncpy(passwd, enc, max);
}


