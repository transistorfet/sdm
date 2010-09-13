/*
 * Object Name:	string.c
 * Description:	String Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/actions/action.h>

MooObjectType moo_string_obj_type = {
	NULL,
	"string",
	typeid(MooString).name(),
	(moo_type_create_t) moo_string_create
};

MooObject *moo_string_create(void)
{
	return(new MooString(""));
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
	memory_free(m_str);
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

int MooString::parse_arg(MooThing *user, MooThing *channel, char *text)
{
	MooArgs::parse_word(text);
	this->set(text);
	return(m_len);
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
		memory_free(m_str);
	if (!str) {
		m_str = "";
		return(0);
	}

	m_len = strlen(str);
	if (!(m_str = (char *) memory_alloc(m_len + 1))) {
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

