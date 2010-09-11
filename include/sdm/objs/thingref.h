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

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int parse_arg(MooThing *user, MooThing *channel, char *text);
	virtual int to_string(char *buffer, int max);

	MooThing *get() { return(MooThing::lookup(m_id)); }
};

extern MooObjectType moo_thingref_obj_type;

MooObject *moo_thingref_create(void);

#endif

