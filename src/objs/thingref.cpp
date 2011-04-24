/*
 * Object Name:	thingref.c
 * Description:	Thing Reference Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/args.h>
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
	return(new MooThingRef());
}

MooThingRef::MooThingRef(const char *str)
{
	MooThing *thing;

	if (!(thing = MooThing::reference(str)))
		throw MooException("Invalid thing reference: %s", str);
	m_id = thing->id();
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

int MooThingRef::to_string(char *buffer, int max)
{
	return(snprintf(buffer, max, "#%d", m_id));
}

MooObject *MooThingRef::access_property(const char *name, MooObject *value)
{
	MooThing *thing;

	if (!(thing = this->get()))
		return(NULL);
	return(thing->access_property(name, value));
}

MooObject *MooThingRef::access_method(const char *name, MooObject *value)
{
	MooThing *thing;

	if (!(thing = this->get()))
		return(NULL);
	return(thing->access_method(name, value));
}

