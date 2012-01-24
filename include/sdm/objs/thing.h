/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_OBJS_THING_H
#define _SDM_OBJS_THING_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/mutable.h>

class MooThing : public MooMutable {
    protected:
	moo_id_t m_parent;
	moo_id_t m_owner;
	moo_mode_t m_mode;
	MooObjectHash *m_properties;
	MooObjectHash *m_methods;

    public:
	MooThing(moo_id_t id = -1, moo_id_t parent = -1, moo_id_t owner = -1, moo_mode_t mode = MOO_DEFAULT_MODE);
	virtual ~MooThing();
	MooThing *clone(moo_id_t id = MOO_NEW_ID);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_object(MooDataFile *data);

	virtual int to_string(char *buffer, int max);
	MooThing *get_thing() { return(this); }

    public:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

    public:
	/// Accessors
	inline moo_id_t parent_id() { return(m_parent); }
	inline MooThing *parent() { return(dynamic_cast<MooThing *>(MooMutable::lookup(m_parent))); }
	moo_id_t owner() { return(m_owner); }
	moo_mode_t mode() { return(m_mode); }

    public:
	void check_throw(moo_mode_t check, moo_id_t owner, moo_mode_t mode);
	int check(moo_mode_t mode, moo_id_t owner, moo_mode_t mode);

	const char *name();
	static MooThing *get_channel(const char *name);
	void quit();
};

extern MooObjectType moo_thing_obj_type;

int init_thing(void);
void release_thing(void);
MooObject *load_moo_thing(MooDataFile *data);

#endif

