/*
 * Object Name:	array.cpp
 * Description:	Array Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/objs/object.h>
#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/thing.h>
#include <sdm/array.h>

struct MooObjectType moo_array_obj_type = {
	NULL,
	"array",
	typeid(MooObjectArray).name(),
	(moo_type_create_t) moo_array_create
};

MooObject *moo_array_create(void)
{
	return(new MooObjectArray(MOO_ARRAY_DEFAULT_SIZE, -1, MOO_OBJECT_ARRAY_DEFAULT_BITS));
}

MooObjectArray::MooObjectArray(int size, int max, int bits) : MooArray<MooObject *>(size, max, bits, (void (*)(MooObject *)) MooGC::decref)
{
	/// Nothing to be done.  This is just here for the call to the MooArray constructor
}

int MooObjectArray::read_entry(const char *type, MooDataFile *data)
{
	int res, index;
	MooObject *obj = NULL;
	char buffer[STRING_SIZE];
	const MooObjectType *objtype;

	// TODO read/write the other values?? like max?
	if (!strcmp(type, "entry")) {
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, NULL))) {
			moo_status("ARRAY: Unable to find entry type, %s", buffer);
			return(-1);
		}
		index = data->read_attrib_integer("index");
		if (!(obj = moo_make_object(objtype))) {
			moo_status("ARRAY: Error loading entry, %d", index);
			return(-1);
		}
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set(index, obj) < 0)) {
			MOO_DECREF(obj);
			return(-1);
		}
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooObjectArray::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	for (int i = 0; i < m_size; i++) {
		if (m_data[i]) {
			data->write_begin_entry("entry");
			data->write_attrib_integer("index", i);
			data->write_attrib_string("type", m_data[i]->type_name());
			m_data[i]->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

int MooObjectArray::parse_arg(MooThing *user, MooThing *channel, char *text)
{

}

int MooObjectArray::to_string(char *buffer, int max)
{

}

MooObject *MooObjectArray::get(int index, MooObjectType *type)
{
	MooObject *obj;

	if (!(obj = MooArray<MooObject *>::get(index)))
		return(NULL);
	if (type && !obj->is_a(type))
		throw moo_type_error;
	return(obj);
}

long int MooObjectArray::get_integer(int index)
{
	MooInteger *obj;

	if (!(obj = (MooInteger *) this->get(index, &moo_integer_obj_type)))
		return(0);
	return(obj->m_num);
}

double MooObjectArray::get_float(int index)
{
	MooFloat *obj;

	if (!(obj = (MooFloat *) this->get(index, &moo_float_obj_type)))
		return(0);
	return(obj->m_num);
}

const char *MooObjectArray::get_string(int index)
{
	MooString *obj;

	if (!(obj = (MooString *) this->get(index, &moo_string_obj_type)))
		return(NULL);
	return(obj->m_str);
}

MooThing *MooObjectArray::get_thing(int index)
{
	MooThingRef *obj;

	if (!(obj = (MooThingRef *) this->get(index, &moo_thingref_obj_type)))
		return(NULL);
	return(obj->get());
}


