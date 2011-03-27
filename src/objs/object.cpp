/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/things/thing.h>
#include <sdm/tasks/task.h>
#include <sdm/objs/object.h>

#define TYPE_INIT_SIZE		32

const MooObjectType moo_object_obj_type = {
	NULL,
	"object",
	typeid(MooObject).name(),
	NULL
};

extern MooObjectHash *global_env;

static MooHash<const MooObjectType *> *name_type_list = NULL;
static MooHash<const MooObjectType *> *realname_type_list = NULL;

int init_object(void)
{
	if (name_type_list)
		return(1);
	name_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	realname_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	moo_object_register_type(&moo_object_obj_type);
	return(0);
}

void release_object(void)
{
	if (!name_type_list)
		return;
	delete realname_type_list;
	delete name_type_list;
	name_type_list = NULL;
}

int moo_object_register_type(const MooObjectType *type)
{
	name_type_list->set(type->m_name, type);
	realname_type_list->set(type->m_realname, type);
	return(0);
}

int moo_object_deregister_type(const MooObjectType *type)
{
	name_type_list->remove(type->m_name);
	realname_type_list->remove(type->m_realname);
	return(0);
}

const MooObjectType *moo_object_find_type(const char *name, const MooObjectType *base)
{
	const MooObjectType *type, *cur;

	if (!(type = name_type_list->get(name)))
		return(NULL);
	/// If base is given, then only return this type if it is a subclass of base
	if (!base)
		return(type);
	for (cur = type; cur; cur = cur->m_parent) {
		if (cur == base)
			return(type);
	}
	return(NULL);
}


MooObject *moo_make_object(const MooObjectType *type)
{
	if (!type)
		return(NULL);
	try {
		return(type->m_create());
	} catch(...) {
		return(NULL);
	}
}

MooObject::MooObject()
{
	m_delete = 0;
	m_owner = MooTask::current_owner();
	m_permissions = MOO_DEFAULT_PERMS;
}

const MooObjectType *MooObject::type()
{
	const MooObjectType *type;
	type = realname_type_list->get(typeid(*this).name());
	if (!type)
		type = name_type_list->get("object");
	return(type);
}

int MooObject::read_file(const char *file, const char *type)
{
	int res = -1;
	MooDataFile *data;

	try {
		data = new MooDataFile(file, MOO_DATA_READ, type);
		moo_status("Reading %s data from file \"%s\".", type, file);
		res = this->read_data(data);
		delete data;
		return(res);
	}
	catch (...) {
		moo_status("Error opening file \"%s\".", file);
		return(-1);
	}
}

int MooObject::write_file(const char *file, const char *type)
{
	MooDataFile *data;

	moo_status("Writing %s data to file \"%s\".", type, file);
	data = new MooDataFile(file, MOO_DATA_WRITE, type);
	this->write_data(data);
	delete data;
	return(0);
}

int MooObject::read_data(MooDataFile *data)
{
	int res;
	int error = 0;
	const char *type;

	do {
		if (!(type = data->read_name()))
			break;
		this->read_entry(type, data);
		if (res < 0)
			error = 1;
		/// We handled the whole rest of the data so just exit
		else if (res == MOO_HANDLED_ALL)
			return(error);
	} while (data->read_next());
	/// We return if the file loaded incorrectly but we don't stop trying to load the file
	return(error);
}

int MooObject::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "owner")) {
		moo_id_t id = data->read_integer_entry();
		this->owner(id);
	}
	else if (!strcmp(type, "permissions")) {
		moo_perm_t perms = data->read_integer_entry();
		this->permissions(perms);
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}

int MooObject::write_data(MooDataFile *data)
{
	data->write_integer_entry("owner", this->owner());
	data->write_octal_entry("permissions", this->permissions());
	return(0);
}

void MooObject::check_throw(moo_perm_t perms)
{
	if (!this->check(perms))
		throw moo_permissions;
}

int MooObject::check(moo_perm_t perms)
{
	int tmp;
	moo_id_t current;

	current = MooTask::current_owner();
	if (MooThing::is_wizard(current))
		return(1);
	if (m_owner == MooTask::current_owner())
		perms <<= 3;
	tmp = m_permissions & perms;
	if (tmp == perms)
		return(1);
	return(0);
}


MooObject *MooObject::resolve(const char *name, MooObjectHash *env, MooObject *value)
{
	MooObject *obj;
	char *action_name, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	if ((action_name = strchr(buffer, ':'))) {
		*action_name = '\0';
		action_name++;
	}

	if ((remain = strchr(buffer, '.'))) {
		*remain = '\0';
		remain++;
	}

	if (!(obj = MooThing::reference(buffer))
	    && !(obj = env->get(buffer, NULL))
	    && !(obj = global_env->get(buffer, NULL)))
		return(NULL);
	if (remain && !(obj = obj->resolve_property(remain, value)))
		return(NULL);

	if (action_name) {
		MooThing *thing;
		if (!(thing = obj->get_thing()))
			return(NULL);
		return(thing->access_method(action_name, value));
	}
	return(obj);
}

MooObject *MooObject::resolve_property(const char *name, MooObject *value)
{
	MooObject *obj;
	char *str, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	// TODO do you need to do permissions checks during all of this?? (should be done in subfunc?)
	// TODO should you throw an error if we can't set the value, instead of just returing NULL?
	for (str = buffer, obj = this; str && *str != '\0' && obj; str = remain) {
		if ((remain = strchr(buffer, '.'))) {
			*remain = '\0';
			remain++;
			if (!(obj = obj->access_property(str, NULL)))
				return(NULL);
		}
		else
			return(obj->access_property(str, value));
	}
	return(NULL);
}


moo_id_t MooObject::owner(moo_id_t id)
{
	return(m_owner = id);
}

moo_perm_t MooObject::permissions(moo_perm_t perms)
{
	// TODO this really sucks.  perms of 0 are invalid.  This is here because when you load perms from XML, if the
	//	tag is not there, it will read as 0, but if perms are not specified, we should assume default_perms
	if (!perms)
		perms = MOO_DEFAULT_PERMS;
	return(m_permissions = perms);
}



