/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/objs/boolean.h>
#include <sdm/objs/string.h>
#include <sdm/objs/funcptr.h>
#include <sdm/objs/thing.h>

#include <sdm/code/code.h>

#define TYPE_INIT_SIZE		32

extern MooObjectHash *global_env;

static MooHash<const MooObjectType *> *name_type_list = NULL;
static MooHash<const MooObjectType *> *realname_type_list = NULL;

int init_object(void)
{
	if (name_type_list)
		return(1);
	name_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	realname_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	return(0);
}

void release_object(void)
{
	if (!name_type_list)
		return;
	delete realname_type_list;
	delete name_type_list;
	name_type_list = NULL;
	realname_type_list = NULL;
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

const MooObjectType *moo_object_find_type(const char *name)
{
	return(name_type_list->get(name));
}

/************************
 * MooObject Definition *
 ************************/

MooObject::MooObject()
{
	m_bitflags = 0;
}

#define DEBUG_LIMIT	77

void MooObject::print_debug()
{
	char buffer[STRING_SIZE];

	//MooCodeParser::generate(m_expr, buffer, LARGE_STRING_SIZE, &moo_style_one_line);
	if (this->to_string(buffer, DEBUG_LIMIT) >= DEBUG_LIMIT)
		strcpy(&buffer[DEBUG_LIMIT], "...");
	moo_status("DEBUG: (%s) %s", typeid(*this).name(), buffer);
}

const MooObjectType *MooObject::objtype()
{
	const MooObjectType *type;
	type = realname_type_list->get(typeid(*this).name());
	if (!type)
		type = name_type_list->get("object");
	return(type);
}

int MooObject::is_true()
{
	if (this == &moo_false)
		return(0);
	return(1);
}

int MooObject::read_data(MooDataFile *data)
{
	int res;
	int error = 0;
	const char *type;

	do {
		type = NULL;
		try {
			if (!(type = data->read_name()))
				break;
			res = this->read_entry(type, data);
			if (res < 0)
				error = 1;
			/// We handled the whole rest of the data so just exit
			else if (res == MOO_HANDLED_ALL)
				return(error);
		}
		catch (MooException e) {
			if (type)
				moo_status("DATA: (at <%s>) %s", type, e.get());
			else
				moo_status("DATA: %s", e.get());
			return(-1);
		}
	} while (data->read_next());
	/// We return if the file loaded incorrectly but we don't stop trying to load the file
	return(error);
}

MooObject *MooObject::read_object(MooDataFile *data, const char *type)
{
	int res;
	MooObject *obj = NULL;
	const MooObjectType *objtype;

	if (!(objtype = moo_object_find_type(type)) || !objtype->m_load) {
		moo_status("OBJ: No such type, %s", type);
		return(NULL);
	}
	res = data->read_children();
	try {
		obj = objtype->m_load(data);
	} catch(MooException e) {
		moo_status("OBJ: Error loading object of type %s", type);
	}
	if (res)
		data->read_parent();
	return(obj);
}

MooObject *MooObject::resolve_property(const char *name, MooObject *value)
{
	MooObject *obj;
	char *str, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

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

MooObject *MooObject::resolve_method(const char *name, MooObject *value)
{
	MooObject *func;

	if (!(func = this->access_method(name, value)))
		return(NULL);
	return(func);
}

