/*
 * Object Name:	boolean.cpp
 * Description:	Boolean Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/boolean.h>

struct MooObjectType moo_boolean_obj_type = {
	NULL,
	"boolean",
	typeid(MooBoolean).name(),
	(moo_type_make_t) make_moo_boolean
};

MooObject *make_moo_boolean(MooDataFile *data)
{
	MooBoolean *obj = new MooBoolean(B_FALSE);
	if (data)
		obj->read_data(data);
	return(obj);
}

int MooBoolean::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "value")) {
		char buffer[STRING_SIZE];

		data->read_string_entry(buffer, STRING_SIZE);
		if (buffer[0] == '#' && buffer[1] != '\0' && buffer[2] == '\0') {
			if (buffer[1] == 't' || buffer[1] == 'T')
				m_bool = B_TRUE;
			else if (buffer[1] == 'f' || buffer[1] == 'f')
				m_bool = B_FALSE;
			else
				moo_status("Error reading boolean at \'%s\'", buffer);
		}
		else
			moo_status("Error reading boolean at \'%s\'", buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooBoolean::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	if (!m_bool)
		data->write_string_entry("value", "#f");
	else
		data->write_string_entry("value", "#t");
	return(0);
}

int MooBoolean::to_string(char *buffer, int max)
{
	if (!m_bool)
		return(snprintf(buffer, max, "#f"));
	else
		return(snprintf(buffer, max, "#t"));
}


