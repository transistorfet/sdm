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


MooThing *user_make_guest(const char *name)
{
	MooThing *user, *parent;

	if (user_exists(name))
		throw MooException("User already exists.  Cannot make guest.");
	if (!(parent = dynamic_cast<MooThing *>(MooObject::resolve("user", NULL))))
		throw moo_thing_not_found;
	user = new MooThing(MOO_NEW_ID, parent->id());
	user->resolve_property("name", new MooString(name));
	user->owner(user->m_id);
	user->resolve_property("description", new MooString("You see a new person who looks rather out-of-place."));
	user->move(dynamic_cast<MooThing *>(MooObject::resolve("start-room", NULL)));
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


