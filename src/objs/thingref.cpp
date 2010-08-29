/*
 * Object Name:	thingref.c
 * Description:	Thing Reference Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

struct MooObjectType moo_thingref_obj_type = {
	NULL,
	"thingref",
	typeid(MooThingRef).name(),
	(moo_type_create_t) moo_thingref_create
};

MooObject *moo_thingref_create(void)
{
	return(new MooThingRef(-1));
}

int MooThingRef::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		m_id = data->read_integer_entry();
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooThingRef::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_integer_entry("value", m_id);
	return(0);
}

