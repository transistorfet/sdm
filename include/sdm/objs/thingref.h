/*
 * Object Name:	thingref.h
 * Description:	Thing Reference Object
 */

#ifndef _SDM_OBJS_THINGREF_H
#define _SDM_OBJS_THINGREF_H

#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

class MooThingRef : public MooObject {
    public:
	moo_id_t m_id;

	MooThingRef(moo_id_t id = -1) { m_id = id; }
	MooThingRef(MooThing *thing) { if (thing) m_id = thing->id(); else m_id = -1; }
	MooThingRef(const char *str);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual MooThing *get_thing() { return(this->get()); }

	MooThing *get() { return(MooThing::lookup(m_id)); }

    private:
	MooObject *access_property(const char *name, MooObject *value = NULL);
	MooObject *access_method(const char *name, MooObject *value = NULL);
};

extern MooObjectType moo_thingref_obj_type;

MooObject *make_moo_thingref(MooDataFile *data);

#endif

