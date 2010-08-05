/*
 * Object Name:	user.cpp
 * Description:	User Object
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
	(moo_type_create_t) moo_user_create
};

static MooHash<MooUser *> *user_list = NULL;

int init_user(void)
{
	const char *str;
	char buffer[STRING_SIZE];
	MooDataFile *data;

	if (user_list)
		return(1);
	user_list = new MooHash<MooUser *>(USER_INIT_SIZE, MOO_HBF_REMOVE | MOO_HBF_DELETEALL);

	/** Load all the users into memory in a disconnected state */
/*
	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib("name", buffer, STRING_SIZE);
			// TODO make sure the user is in it's proper disconnected state (in the cryolocker)
			//create_sdm_user(buffer);
		}
	} while (data->read_next());
	delete data;
*/
	return(0);
}

void release_user(void)
{
	delete user_list;
	user_list = NULL;
}

MooObject *moo_user_create(void)
{
	return(new MooUser());
}


MooUser::MooUser(const char *name, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_task = NULL;
	this->load(name);
}

MooUser::~MooUser()
{
	/** Save the user information to the user's file, and disconnect */
	this->disconnect();

	/** Release the user's other resources */
	user_list->remove(m_name);
	memory_free(m_name);
	//delete m_inter;
}

int MooUser::load(const char *name)
{
	if (!name || !moo_user_valid_username(name) || !(m_name = make_string("%s", name)))
		throw -1;

	/** If there is already a user with that name then fail */
	if (user_list->set(name, this))
		return(-1);
	this->read_file();
	return(0);
}

int MooUser::connect(MooInterface *inter)
{
	double room;
	MooThing *location;

/*
	if (m_inter)
		delete m_inter;
	m_inter = inter;

	// TODO we could instead of all this crap, have the m_task set by the task who manipulates this object
	//	All output would be written to the task via an output method called from whatever code (possibly all
	//	code would call user->output() or write() or whatever, and it would then call m_task->output() which would
	//	then call output() on the interface.

	if (m_parent > 0)
		m_task = new MooInterpreter();
	else
		m_task = new MooForm("etc/register.xml");

	// Move the user to the last location recorded or to a safe place if there is no last location
	if (((room = sdm_get_number_property(SDM_THING(user), "last_location")) > 0)
	    && (location = moo_thing_lookup_id(room)))
		sdm_moveto(SDM_THING(user), SDM_THING(user), location, NULL);
	else
		// TODO you should do this some othe way
		sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_thing_lookup_id(50), NULL);
		//sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_interpreter_find_object(NULL, "/lost+found"), NULL);

	if (!user->m_task)
		return(-1);
	user->m_task->initialize(user);
*/
	return(0);
}

void MooUser::disconnect()
{
/*
	struct sdm_number *number;

	// Shutdown the input processor
	if (user->proc) {
		sdm_processor_shutdown(user->proc, user);
		destroy_sdm_object(SDM_OBJECT(user->proc));
		user->proc = NULL;
	}

	if ((number = SDM_NUMBER(sdm_thing_get_property(SDM_THING(user), "last_location", &sdm_number_obj_type)))
	    || ((number = create_sdm_number(-1)) && !sdm_thing_set_property(SDM_THING(user), "last_location", SDM_OBJECT(number)))) {
		number->num = SDM_THING(user)->location ? SDM_THING(user)->location->id : -1;
	}
	// TODO how do you tell this function to forcefully remove the user
	if (SDM_THING(user)->location)
		sdm_thing_remove(SDM_THING(user)->location, SDM_THING(user));

	// Save the user information to the user's file only if we were already connected
	if (user->inter)
		sdm_user_write(user);
	// TODO How does this all even work?
	//user->inter = NULL;
*/
}


int MooUser::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "name")) {
		// TODO this should already have been set but maybe we can generate an error if it doesn't match
	}
	// TODO read processor type and settings
	// TODO read telnet/interface settings if we are going to do that
	else
		return(MooThing::read_entry(type, data));
	return(0);
}

int MooUser::write_data(MooDataFile *data)
{
	MooThing::write_data(data);
	data->write_string_entry("name", m_name);
	data->write_integer_entry("lastseen", time(NULL));
	// TODO write telnet/interface settings
	// TODO write processor settings and types
	return(0);
}

int MooUser::read_file(void)
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name);
	return(MooObject::read_file(buffer, "user"));
}

int MooUser::write_file(void)
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name);
	return(MooObject::write_file(buffer, "user"));
}


int moo_user_exists(const char *name)
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", name);
	return(moo_data_file_exists(buffer));
}

int moo_user_logged_in(const char *name)
{
	MooUser *user;

	//if ((user = user_list->get(name)) && user->m_inter)
	//	return(1);
	//return(0);
}

int moo_user_valid_username(const char *name)
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

