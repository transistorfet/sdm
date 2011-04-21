/*
 * Object Name:	number.cpp
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
	return(new MooNumber((long int) 0));
}

MooNumber::MooNumber(const char *str)
{
	char *endptr;

	if (strchr(str, '.')) {
		m_format = FLOAT;
		m_float = strtod(str, &endptr);
	}
	else {
		m_format = INT;
		m_int = strtol(str, &endptr, 0);
	}
	if (*endptr != '\0')
		throw MooException("Invalid number, %s", str);
}

int MooNumber::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		char buffer[STRING_SIZE];

		data->read_attrib_string("format", buffer, STRING_SIZE);
		if (!strcmp(buffer, "int")) {
			m_format = INT;
			m_int = data->read_integer_entry();
		}
		else if (!strcmp(buffer, "float")) {
			m_format = FLOAT;
			m_float = data->read_float_entry();
		}
		else
			throw MooException("Invalid number format type");
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooNumber::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	if (m_format == INT)
		data->write_integer_entry("value", m_int);
	else if (m_format == FLOAT)
		data->write_float_entry("value", m_float);
	return(0);
}

int MooNumber::to_string(char *buffer, int max)
{
	if (m_format == INT)
		return(snprintf(buffer, max, "%ld", m_int));
	else if (m_format == FLOAT)
		return(snprintf(buffer, max, "%f", m_float));
	return(0);
}

void MooNumber::set_format(MooNumberFormatT format)
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


