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

#define MOO_IS_WHITESPACE(ch)	( (ch) == ' ' || (ch) == '\n' || (ch) == '\r' )

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

	if (!name || !MooUser::valid_username(name))
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

/*
	// Move the user to the last location recorded or to a safe place if there is no last location
	if (((room = sdm_get_number_property(SDM_THING(user), "last_location")) > 0)
	    && (location = moo_thing_lookup_id(room)))
		sdm_moveto(SDM_THING(user), SDM_THING(user), location, NULL);
	else
		// TODO you should do this some othe way
		sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_thing_lookup_id(50), NULL);
		//sdm_moveto(SDM_THING(user), SDM_THING(user), sdm_interpreter_find_object(NULL, "/lost+found"), NULL);
*/
	// If an error occurs and we return early, m_task will not be set, so if disconnect() is called, we wont save the user
	m_task = task;
	return(0);
}

void MooUser::disconnect()
{
	//if ((number = SDM_NUMBER(sdm_thing_get_property(SDM_THING(user), "last_location", &sdm_number_obj_type)))
	//    || ((number = create_sdm_number(-1)) && !sdm_thing_set_property(SDM_THING(user), "last_location", SDM_OBJECT(number)))) {
	//	number->num = SDM_THING(user)->location ? SDM_THING(user)->location->id : -1;
	//}

	// TODO make sure this will work; it's a superb improvement over the old way we had to do this (note: any old value would be deleted)
	// TODO we want this to be treated as a MooThingRef and not a float number
	//this->set_property("last_location", m_location ? m_location->m_id : -1);

	// TODO how do you tell this function to forcefully remove the user
	//if (SDM_THING(user)->location)
	//	sdm_thing_remove(SDM_THING(user)->location, SDM_THING(user));
	// TODO call action "force_exit" or something on location to remove user
	//cryolocker->add(this);

	// Save the user information to the user's file only if we were already connected
	if (m_task) {
		// TODO don't save while testing (works now but still wont)
		//this->save();
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
	// TODO wait, would we even need to save these things?
	// TODO write telnet/interface settings
	// TODO write processor settings and types
	return(0);
}

int MooUser::command(const char *text)
{
	int i;
	char *action;
	MooThing *object, *target;
	char buffer[LARGE_STRING_SIZE];

	strcpy(buffer, text);
	// Parse out the action string
	for (i = 0; buffer[i] != '\0' && MOO_IS_WHITESPACE(buffer[i]); i++)
		;
	action = &buffer[i];
	for (; buffer[i] != '\0' && !MOO_IS_WHITESPACE(buffer[i]); i++)
		;
	buffer[i] = '\0';

	// TODO should we break the text into words first (which will also process out any quoted strings)
	// split the text into 2 strings by looking for a preposition (???)
	// call user->find_thing() on the 2 object names or NULL if not found



	return(this->command(action, object, target));
}

int MooUser::command(const char *action, const char *text)
{
	// TODO this is used by IRC to do command("say", <text>);

	return(0);
}

int MooUser::command(const char *action, MooThing *object, MooThing *target)
{
	int res;
	MooArgs args;

	// TODO change this with a MooArgs method for setting
	args.m_user = this;
	args.m_caller = (MooThing *) this;
	args.m_object = object;
	args.m_target = target;
	args.m_text = NULL;

	if ((res = this->do_action(action, &args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (m_location && (res = m_location->do_action(action, &args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (object && (res = object->do_action(action, &args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (target && (res = target->do_action(action, &args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	return(MOO_ACTION_NOT_FOUND);
}

MooThing *MooUser::find_thing(const char *name)
{
	MooThing *thing;

	if ((thing = MooThing::reference(name)))
		return(thing);
	else if (!strcmp(name, "me"))
		return(this);
	else if (!strcmp(name, "here") && m_location)
		return(m_location);
	else if ((thing = this->find(name)))
		return(thing);
	else if (m_location && (thing = m_location->find(name)))
		return(thing);
	return(NULL);
}

int MooUser::output(MooThing *thing, const char *text, ...)
{
	// TODO this should be passed *at least* one 'thing' pointer to identify the source
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

MooUser *MooUser::register_new(const char *name, ...)
{
	// TODO create a new user (don't know what info we will need for this)
	return(NULL);
}

MooUser *MooUser::login(const char *name, const char *passwd)
{
	MooUser *user;

	if (!(user = user_list->get(name)) || user->m_task)
		return(NULL);
	// TODO how the fuck do we get the password!?  It's got to come from etc/passwd.xml
	//if (strcmp(user->m_passwd, passwd))
	//	return(NULL);

	// TODO should this also do connect()
	return(user);
}

void MooUser::encrypt_password(const char *salt, char *passwd, int max)
{
	char *enc;

	enc = crypt(passwd, salt);
	strncpy(passwd, enc, max);
}


