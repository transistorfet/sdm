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
	return(new MooInteger((long int) 0));
}

MooInteger::MooInteger(const char *str)
{
	char *endptr;

	if (strchr(str, '.')) {
		m_format = FLOAT;
		m_float = strtod(str, &endptr);
	}
	else {
		m_format = FLOAT;
		m_int = strtol(str, &endptr, 0);
	}
	if (*endptr != '\0')
		throw MooException("Invalid integer, %s", str);
}

int MooInteger::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		if (m_format == INT)
			m_int = data->read_integer_entry();
		else if (m_format == FLOAT)
			m_float = data->read_float_entry();
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooInteger::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	if (m_format == INT)
		data->write_integer_entry("value", m_int);
	else if (m_format == FLOAT)
		data->write_float_entry("value", m_float);
	return(0);
}

int MooInteger::to_string(char *buffer, int max)
{
	if (m_format == INT)
		return(snprintf(buffer, max, "%ld", m_int));
	else if (m_format == FLOAT)
		return(snprintf(buffer, max, "%f", m_float));
	return(0);
}

void MooInteger::set_format(MooNumberFormatT format)
{
	if (format == FLOAT && m_format == INT) {
		m_format = FLOAT;
		m_float = (double) m_int;
	}
	else if (format == INT && m_format == FLOAT) {
		m_format = INT;
		m_int = (long int) m_float;
	}
}


