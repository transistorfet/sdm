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
	m_id = data->read_integer();
	return(MOO_HANDLED_ALL);
}

int MooThingRef::write_data(MooDataFile *data)
{
	data->write_integer(m_id);
	return(0);
}

