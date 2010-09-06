/*
 * Object Name:	hash.cpp
 * Description:	Hash Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/thing.h>
#include <sdm/hash.h>

struct MooObjectType moo_hash_obj_type = {
	NULL,
	"hash",
	typeid(MooObjectHash).name(),
	(moo_type_create_t) moo_hash_create
};

MooObject *moo_hash_create(void)
{
	return(new MooObjectHash(MOO_HASH_DEFAULT_SIZE, MOO_HBF_DELETE | MOO_HBF_DELETEALL | MOO_HBF_REPLACE | MOO_HBF_REMOVE));
}

MooObjectHash::MooObjectHash(int size, int bits) : MooHash<MooObject *>(size, bits)
{
	/// Nothing to be done.  This is just here for the call to the MooArray constructor
}

int MooObjectHash::read_entry(const char *type, MooDataFile *data)
{
	int res;
	MooObject *obj = NULL;
	char key[STRING_SIZE];
	char buffer[STRING_SIZE];
	const MooObjectType *objtype;

	if (!strcmp(type, "entry")) {
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, NULL)))
			return(-1);
		if (!(obj = moo_make_object(objtype)))
			return(-1);
		data->read_attrib_string("key", key, STRING_SIZE);
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set(key, obj) < 0)) {
			delete obj;
			return(-1);
		}
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooObjectHash::write_data(MooDataFile *data)
{
	MooHashEntry<MooObject *> *cur;

	MooObject::write_data(data);
	this->reset();
	while ((cur = this->next_entry())) {
		data->write_begin_entry("entry");
		data->write_attrib_string("key", cur->m_key);
		data->write_attrib_string("type", cur->m_data->type_name());
		cur->m_data->write_data(data);
		data->write_end_entry();
	}
	return(0);
}

MooObject *MooObjectHash::get(const char *key, MooObjectType *type)
{
	MooObject *obj;

	if (!(obj = MooHash<MooObject *>::get(key)))
		return(NULL);
	if (!obj->is_a(type))
		return(NULL);
	return(obj);
}

double MooObjectHash::get_number(const char *key)
{
	MooNumber *obj;

	// TODO should this throw an exception rather than just return 0 on error??
	if (!(obj = (MooNumber *) this->get(key, &moo_number_obj_type)))
		return(0);
	return(obj->m_num);
}

const char *MooObjectHash::get_string(const char *key)
{
	MooString *obj;

	// TODO should this throw an exception rather than just return NULL on error??
	if (!(obj = (MooString *) this->get(key, &moo_string_obj_type)))
		return(NULL);
	return(obj->m_str);
}

MooThing *MooObjectHash::get_thing(const char *key)
{
	MooThingRef *obj;

	if (!(obj = (MooThingRef *) this->get(key, &moo_thingref_obj_type)))
		return(NULL);
	return(obj->get());
}


