/*
 * Object Name:	boolean.cpp
 * Description:	Boolean Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/boolean.h>

MooBoolean moo_true = MooBoolean(B_TRUE);
MooBoolean moo_false = MooBoolean(B_FALSE);

struct MooObjectType moo_boolean_obj_type = {
	"boolean",
	typeid(MooBoolean).name(),
	(moo_type_load_t) load_moo_boolean
};

MooObject *load_moo_boolean(MooDataFile *data)
{
	char buffer[STRING_SIZE];

	data->read_string_entry(buffer, STRING_SIZE);
	if (buffer[0] == '#' && buffer[1] != '\0' && buffer[2] == '\0') {
		if (buffer[1] == 't' || buffer[1] == 'T')
			return(&moo_true);
		else if (buffer[1] == 'f' || buffer[1] == 'f')
			return(&moo_false);
	}
	return(NULL);
}

int MooBoolean::read_entry(const char *type, MooDataFile *data)
{
	// TODO is this now not needed?
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


