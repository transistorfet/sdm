/*
 * Object Name:	float.c
 * Description:	Float Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/float.h>

struct MooObjectType moo_float_obj_type = {
	NULL,
	"float",
	typeid(MooFloat).name(),
	(moo_type_create_t) moo_float_create
};

MooObject *moo_float_create(void)
{
	return(new MooFloat((moo_float_t) 0));
}

MooFloat::MooFloat(const char *str)
{
	char *endptr;

	m_num = strtod(str, &endptr);
	if (*endptr != '\0')
		throw MooException("Invalid float, %s", str);
}

int MooFloat::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		m_num = data->read_float_entry();
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooFloat::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_float_entry("value", m_num);
	return(0);
}

int MooFloat::to_string(char *buffer, int max)
{
	return(snprintf(buffer, max, "%f", m_num));
}

