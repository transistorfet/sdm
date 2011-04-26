/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_THINGS_THING_H
#define _SDM_THINGS_THING_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/things/references.h>

#define MOO_ACTION_NOT_FOUND	1

#define MOO_NO_ID		-1
#define MOO_NEW_ID		-2

#define MOO_TBF_WIZARD		0x01

extern MooArray<MooThing *> *moo_thing_table;

class MooThing : public MooObject {
    protected:
	int m_bits;
	moo_id_t m_id;
	moo_id_t m_parent;
	MooThing *m_location;
	MooObjectHash *m_properties;
	MooObjectHash *m_methods;

	MooThing *m_next;
	MooThing *m_objects;
	MooThing *m_end_objects;

    public:
	MooThing(moo_id_t id = -1, moo_id_t parent = 0);
	virtual ~MooThing();
	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual MooThing *get_thing() { return(this); }

    public:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

    private:
	// TODO integrate these directly into the access* methods?
	int set_property(const char *name, MooObject *obj);
	MooObject *get_property(const char *name, MooObjectType *type);
	MooObject *get_property_raw(const char *name, MooThing **thing);
	int set_method(const char *name, MooObject *action);
	MooObject *get_method(const char *name);

    public:
	/// Search Methods
	MooThing *find(const char *name);
	MooThing *find_named(const char *name);
	static MooThing *reference(const char *name);

	/// Helper Methods
	static int attach_orphans();

	int is_wizard() { return(m_bits & MOO_TBF_WIZARD); }
	static int is_wizard(moo_id_t id);

    public:
	/// Accessors
	inline moo_id_t id() { return(m_id); }
	inline moo_id_t parent_id() { return(m_parent); }
	inline MooThing *parent() { return(MooThing::lookup(m_parent)); }
	inline MooThing *owner_thing() { return(MooThing::lookup(this->owner())); }
	inline int is_a_thing(moo_id_t id);
	inline int is_a_thing(const char *name);
	static inline MooThing *lookup(moo_id_t id) { return(moo_thing_table->get(id)); }

	const char *name();
	MooThing *location();
	inline MooObjectArray *contents() { return(dynamic_cast<MooObjectArray *>(this->resolve_property("contents"))); }
	int move(MooThing *where);

    protected:
	int assign_id(moo_id_t id);

	int add(MooThing *thing);
	int remove(MooThing *thing);
};

extern MooObjectType moo_thing_obj_type;

int init_thing(void);
void release_thing(void);
MooObject *moo_thing_create(void);

inline int MooThing::is_a_thing(moo_id_t id)
{
	for (MooThing *cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if (cur->m_id == id)
			return(1);
	}
	return(0);
}

#endif

