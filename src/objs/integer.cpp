/*
 * Object Name:	integer.cpp
 * Description:	Integer Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/integer.h>

struct MooObjectType moo_integer_obj_type = {
	NULL,
	"integer",
	typeid(MooInteger).name(),
	(moo_type_create_t) moo_integer_create
};

MooObject *moo_integer_create(void)
{
	return(new MooInteger((moo_integer_t) 0));
}

MooInteger::MooInteger(const char *str)
{
	char *endptr;

	m_num = strtol(str, &endptr, 0);
	if (*endptr != '\0')
		throw MooException("Invalid integer, %s", str);
}

int MooInteger::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		m_num = data->read_integer_entry();
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooInteger::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_integer_entry("value", m_num);
	return(0);
}

int MooInteger::parse_arg(MooThing *user, MooThing *channel, char *text)
{
	char *remain;

	m_num = strtol(text, &remain, 0);
	return(remain - text);
}

int MooInteger::to_string(char *buffer, int max)
{
	return(snprintf(buffer, max, "%ld", m_num));
}

