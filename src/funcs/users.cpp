/*
 * Name:	users.cpp
 * Description:	User Methods
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/tasks/task.h>
#include <sdm/objs/thing.h>

#include <sdm/funcs/users.h>


MooThing *user_make_guest(const char *name)
{
	MooThing *user, *parent;

	if (user_exists(name))
		throw MooException("User already exists.  Cannot make guest.");
	if (!(parent = dynamic_cast<MooThing *>(MooObject::resolve("user", NULL))))
		throw moo_thing_not_found;
	user = new MooThing(MOO_NEW_ID, parent->id());
	user->resolve_property("name", new MooString(name));
	//user->owner(user->id());
	user->resolve_property("description", new MooString("You see a new person who looks rather out-of-place."));
	//user->move(dynamic_cast<MooThing *>(MooObject::resolve("start-room", NULL)));
	return(user);
}

int user_logged_in(const char *name)
{
	MooThing *user;
	MooObject *users;
	MooObjectHash *list;

	if (!(users = MooObject::resolve("NickServ", global_env))) {
		moo_status("USER: NickServ object not found");
		return(0);
	}
	if (!(list = dynamic_cast<MooObjectHash *>(users->resolve_property("db")))) {
		moo_status("USER: users list not found in NickServ");
		return(0);
	}
	if (!(user = dynamic_cast<MooThing *>(list->get(name)))) {
		moo_status("USER: No such user in NickServ, %s", name);
		return(0);
	}
	if (dynamic_cast<MooTask *>(user->resolve_property(name)))
		return(1);
	return(0);
}

int user_valid_username(const char *name)
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

MooThing *user_login(const char *name, const char *passwd)
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
				return(MooThing::lookup(id));
			}
			break;
		}
	} while (data->read_next());
	delete data;
	return(NULL);
}

int user_exists(const char *name)
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


void user_encrypt_password(const char *salt, char *passwd, int max)
{
	char *enc;

	enc = crypt(passwd, salt);
	strncpy(passwd, enc, max);
}


static int user_notify(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	int type;
	MooTask *task;
	MooObject *obj;
	MooThing *user, *channel;
	MooThing *m_this;
	char buffer[LARGE_STRING_SIZE];

	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(task = dynamic_cast<MooTask *>(m_this->resolve_property("task"))))
		throw MooException("User not connected; No \"task\" defined.");
	if (args->last() != 4)
		throw moo_args_mismatched;
	type = args->get_integer(1);
	user = args->get_thing(2);
	channel = args->get_thing(3);
	if (!(obj = args->get(4)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (type < TNT_FIRST || type > TNT_LAST)
		throw MooException("arg 1: Invalid notify() type");
	// TODO permissions check!? I guess on task
	task->notify(type, user, channel, buffer);
	return(0);
}

int moo_load_user_methods(MooObjectHash *env)
{
	env->set("N/STATUS", new MooNumber((long int) TNT_STATUS));
	env->set("N/JOIN", new MooNumber((long int) TNT_JOIN));
	env->set("N/LEAVE", new MooNumber((long int) TNT_LEAVE));
	env->set("N/SAY", new MooNumber((long int) TNT_SAY));
	env->set("N/EMOTE", new MooNumber((long int) TNT_EMOTE));
	env->set("N/QUIT", new MooNumber((long int) TNT_QUIT));

	env->set("%user_notify", new MooFunc(user_notify));
	return(0);
}

