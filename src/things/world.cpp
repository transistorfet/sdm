/*
 * Object Name:	world.c
 * Description:	Root World Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>

MooObjectType moo_world_obj_type = {
	&moo_thing_obj_type,
	"world",
	typeid(MooWorld).name(),
	(moo_type_create_t) moo_world_create
};

MooWorld *moo_root_world = NULL;

int init_world(void)
{
	if (moo_root_world)
		return(1);
	// TODO go through all things and write to a root file.
	// TODO alternatively you can pass over all things and fix them so if their location is NULL, add them to the root world
	/// Create the world object with ID = 1, and parent ID = -1 since the world has no parent
	moo_root_world = new MooWorld("maps/world.xml", 1, -1);
	moo_object_register_type(&moo_world_obj_type);
	return(0);
}

void release_world(void)
{
	if (!moo_root_world)
		return;
	/// Write the world to disk
	// TODO BUG there is an infinite loop on writing the world file (??? is there?)
	// TODO commented out while we test MooCode
	//moo_root_world->write();
	delete moo_root_world;
	moo_object_deregister_type(&moo_world_obj_type);
}

MooObject *moo_world_create(void)
{
	return(new MooWorld());
}

MooWorld::MooWorld()
{
	m_filename = NULL;
	m_writing = 0;
	m_written = 0;
}

MooWorld::MooWorld(const char *file, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_filename = new std::string(file);
	m_writing = 0;
	m_written = 0;
	this->read_file(m_filename->c_str(), "world");
}

MooWorld::~MooWorld()
{
	/// We don't write the world here because we assume the root world is being destroyed and we
	/// don't want to write each world twice so if for some reason a world is being destroyed without
	/// the root world being destroyed, it must be written manually
	if (m_filename)
		delete m_filename;
}

int MooWorld::read_entry(const char *type, MooDataFile *data)
{
	MooWorld *obj;
	char buffer[STRING_SIZE];

	if (!strcmp(type, "load")) {
		data->read_attrib_string("ref", buffer, STRING_SIZE);
		obj = new MooWorld(buffer, MOO_NO_ID, 0);
		if (this->add(obj) < 0) {
			delete obj;
			return(-1);
		}
	}
	else
		return(MooThing::read_entry(type, data));
	return(0);
}


int MooWorld::write_data(MooDataFile *data)
{
	int res;

	if (m_writing) {
		if (m_written)
			return(-1);
		m_written = 1;
		res = MooThing::write_data(data);
	}
	else {
		if (data) {
			data->write_begin_entry("load");
			data->write_attrib_string("ref", m_filename->c_str());
			data->write_end_entry();
		}
		res = this->write();
	}
	return(res);
}

int MooWorld::write()
{
	int res;

	MooThing::attach_orphans();
	if (m_writing)
		return(-1);
	m_writing = 1;
	m_written = 0;

	char buffer[STRING_SIZE];
	strcpy(buffer, m_filename->c_str());
	strncpy(buffer, "temp", 4);

	res = this->write_file(buffer, "world");
	m_writing = 0;
	return(res);
}

MooWorld *MooWorld::root()
{
	return(moo_root_world);
}


