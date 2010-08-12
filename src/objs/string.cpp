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
	char buffer[STRING_SIZE];

	data->read_string(buffer, STRING_SIZE);
	this->set(buffer);
	return(MOO_HANDLED_ALL);
}

int MooString::write_data(MooDataFile *data)
{
	data->write_string(m_str);
	return(0);
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

