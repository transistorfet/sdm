/*
 * Object Name:	world.h
 * Description:	Root World Object
 */

#ifndef _SDM_THINGS_WORLD_H
#define _SDM_THINGS_WORLD_H

#include <string>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

class MooWorld : public MooThing {
	std::string *m_filename;
	int m_writing;
	int m_written;
    public:
	MooWorld();
	MooWorld(const char *file);
	MooWorld(const char *file, moo_id_t id, moo_id_t parent);
	virtual ~MooWorld();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int write();

	static MooWorld *root();
};

extern MooObjectType moo_world_obj_type;

int init_world(void);
void release_world(void);
MooObject *moo_world_create(void);

#endif

