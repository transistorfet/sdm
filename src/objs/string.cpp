/*
 * Object Name:	string.c
 * Description:	String Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>

MooObjectType moo_string_obj_type = {
	NULL,
	"string",
	typeid(MooString).name(),
	(moo_type_make_t) make_moo_string
};

MooObject *make_moo_string(MooDataFile *data)
{
	MooString *obj = new MooString("");
	if (data)
		obj->read_data(data);
	return(obj);
}

MooString::MooString(const char *fmt, ...)
{
	va_list va;
	char buffer[LARGE_STRING_SIZE];

	m_str = NULL;
	m_len = 0;
	va_start(va, fmt);
	vsnprintf(buffer, LARGE_STRING_SIZE, fmt, va);
	va_end(va);
	this->set(buffer);
}

MooString::~MooString()
{
	free(m_str);
}

int MooString::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		char buffer[STRING_SIZE];

		data->read_string_entry(buffer, STRING_SIZE);
		this->set(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooString::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	data->write_string_entry("value", m_str);
	return(0);
}

int MooString::to_string(char *buffer, int max)
{
	strncpy(buffer, m_str, max);
	if (m_len >= max) {
		buffer[max] = '\0';
		return(max);
	}
	return(m_len);
}


int MooString::set(const char *str)
{
	if (m_str)
		free(m_str);
	if (!str) {
		m_str = "";
		return(0);
	}

	m_len = strlen(str);
	if (!(m_str = (char *) malloc(m_len + 1))) {
		m_len = 0;
		return(-1);
	}
	strcpy(m_str, str);
	return(0);
}

int MooString::compare(const char *str)
{
	return(strcasecmp(m_str, str));
}

int MooString::compare(const char *str, int len)
{
	return(strncasecmp(m_str, str, len));
}

