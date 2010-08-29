/*
 * Object Name:	number.c
 * Description:	Number Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/number.h>

struct MooObjectType moo_number_obj_type = {
	NULL,
	"number",
	typeid(MooNumber).name(),
	(moo_type_create_t) moo_number_create
};

MooObject *moo_number_create(void)
{
	return(new MooNumber(0));
}

int MooNumber::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		m_num = data->read_float_entry();
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}

int MooNumber::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_float_entry("value", m_num);
	return(0);
}

