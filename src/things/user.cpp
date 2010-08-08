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
	(moo_type_create_t) NULL
};

static MooThing *cryolocker = NULL;
static MooHash<MooUser *> *user_list = NULL;

int init_user(void)
{
	const char *str;
	char buffer[STRING_SIZE];
	MooDataFile *data;

	if (user_list)
		return(1);
	user_list = new MooHash<MooUser *>(USER_INIT_SIZE, MOO_HBF_REMOVE | MOO_HBF_DELETEALL);
	moo_object_register_type(&moo_user_obj_type);
	// TODO we need to find/create/whatever the cryolocker! (we haven't loaded the world at this point though)

	/** Load all the users into memory in a disconnected state */
	data = new MooDataFile("etc/passwd.xml", MOO_DATA_READ, "passwd");
	do {
		if ((str = data->read_name()) && !strcmp(str, "user")) {
			data->read_attrib("name", buffer, STRING_SIZE);
			new MooUser(buffer);
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


MooUser::MooUser(const char *name, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_task = NULL;
	m_name = NULL;

	if (!name || !moo_user_valid_username(name))
		throw MooException("User name error");
	m_name = new std::string(name);

	if (user_list->set(name, this))
		throw MooException("Unable to add user to list: %s", name);
	this->load();
}

MooUser::~MooUser()
{
	/** Save the user information to the user's file, and disconnect */
	this->disconnect();

	/** If we are associated with a task, then notify it that we are dying */
	if (m_task)
		m_task->purge(this);

	/** Release the user's other resources */
	if (m_name) {
		user_list->remove(m_name->c_str());
		delete m_name;
	}
}

int MooUser::load()
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name->c_str());
	return(MooObject::read_file(buffer, "user"));
}

int MooUser::save()
{
	char buffer[STRING_SIZE];

	snprintf(buffer, STRING_SIZE, "users/%s.xml", m_name->c_str());
	return(MooObject::write_file(buffer, "user"));
}

int MooUser::connect(MooTask *task)
{
	double room;
	MooThing *location;

	if (m_task)
		return(-1);
	m_task = task;



	// TODO we could instead of all this crap, have the m_task set by the task who manipulates this object
	//	All output would be written to the task via an output method called from whatever code (possibly all
	//	code would call user->output() or write() or whatever, and it would then call m_task->output() which would
	//	then call output() on the interface.

/*
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
	//if ((number = SDM_NUMBER(sdm_thing_get_property(SDM_THING(user), "last_location", &sdm_number_obj_type)))
	//    || ((number = create_sdm_number(-1)) && !sdm_thing_set_property(SDM_THING(user), "last_location", SDM_OBJECT(number)))) {
	//	number->num = SDM_THING(user)->location ? SDM_THING(user)->location->id : -1;
	//}

	// TODO make sure this will work; it's a superb improvement over the old way we had to do this (note: any old value would be deleted)
	//this->set_property("last_location", m_location ? m_location->m_id : -1);

	// TODO how do you tell this function to forcefully remove the user
	//if (SDM_THING(user)->location)
	//	sdm_thing_remove(SDM_THING(user)->location, SDM_THING(user));
	// TODO call action "force_exit" or something on location to remove user
	//cryolocker->add(this);

	// Save the user information to the user's file only if we were already connected
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
	// TODO read processor type and settings
	// TODO read telnet/interface settings if we are going to do that
	else
		return(MooThing::read_entry(type, data));
	return(0);
}

int MooUser::write_data(MooDataFile *data)
{
	MooThing::write_data(data);
	data->write_string_entry("name", m_name->c_str());
	data->write_integer_entry("lastseen", time(NULL));
	// TODO wait, would we even need to save these things?
	// TODO write telnet/interface settings
	// TODO write processor settings and types
	return(0);
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


/** Called by the client task which reads input from the interface.  This function would parse
 *  the input as an action and then execute that action. */
/*
int MooUser::command(const char *text)
{

}

int MooUser::command(const char *action, const char *text)
{

}

int MooUser::command(const char *action, const char *object, const char *target)
{

}
*/

int MooUser::command(const char *action, MooThing *object, MooThing *target)
{
	MooArgs args;

	// TODO change this with a MooArgs method for setting
	args.m_user = this;
	args.m_caller = (MooThing *) this;
	args.m_thing = (MooThing *) this;
	args.m_object = object;
	args.m_target = target;
	args.m_text = NULL;

	// TODO should this be abreved action instead?
	return(object->do_action(action, &args));
}

/** Sends data output of some kind to the client.  Things like the output of actions, like 'look', 'exits', etc
 */
int MooUser::tell(const char *text, ...)
{

}


