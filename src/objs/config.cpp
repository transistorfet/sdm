/*
 * Object Name:	config.c
 * Description:	Config Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/config.h>
#include <sdm/tasks/listener.h>

static MooConfig *config = NULL;

int load_global_config()
{
	if (config)
		return(1);
	config = new MooConfig("etc/config.xml");
	return(0);
}

MooConfig::MooConfig(const char *filename)
{
	m_filename = new std::string(filename);
	this->read_file(filename, "config");
}

MooConfig::~MooConfig()
{
	delete m_filename;
}

int MooConfig::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "listener")) {
		try {
			MooListener *listener = new MooListener();
			data->read_children();
			listener->read_data(data);
			data->read_parent();
		}
		catch (MooException e) {
			moo_status("listener error: %s", e.get());
		}
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}

int MooConfig::write_data(MooDataFile *data)
{
//	data->write_string(this->str);
	return(0);
}


