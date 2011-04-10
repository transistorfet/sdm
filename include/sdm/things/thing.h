/*
 * Object Name:	thing.h
 * Description:	Base Game Object
 */

#ifndef _SDM_THINGS_THING_H
#define _SDM_THINGS_THING_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/tree.h>
#include <sdm/data.h>
#include <sdm/array.h>
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

	static MooThing *create(moo_id_t parent);
	MooThing *clone();
	// TODO should this be moved to moo_code??
	static void add_global(const char *name, MooObject *obj);

    private:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

    public:
	// TODO integrate these directly into the access* methods?
	/// Property Methods
	int set_property(const char *name, MooObject *obj);
	MooObject *get_property(const char *name, MooObjectType *type);

	/// Action Methods
	int set_action(const char *name, MooObject *action);
	MooObject *get_action(const char *name);

	static int convert_result(MooObject *&result, int def = 0);

	/// Search Methods
	MooThing *find(const char *name);
	MooThing *find_named(const char *name);
	static MooThing *reference(const char *name);

	/// Helper Methods
	int command(MooThing *user, MooThing *channel, const char *action, const char *text = NULL);
	virtual int notify(int type, MooThing *thing, MooThing *channel, const char *text);
	int notifyf(int type, MooThing *thing, MooThing *channel, const char *fmt, ...);
	int notify(int type, MooArgs *args, const char *fmt, ...);
	int notify_all(int type, MooThing *thing, MooThing *channel, const char *text);
	int notify_all_except(MooThing *except, int type, MooThing *thing, MooThing *channel, const char *text);

	int moveto(MooThing *user, MooThing *channel, MooThing *to);
	static int attach_orphans();

	int is_wizard() { return(m_bits & MOO_TBF_WIZARD); }
	static int is_wizard(moo_id_t id);

    public:
	/// Accessors
	inline moo_id_t id() { return(m_id); }
	inline moo_id_t parent_id() { return(m_parent); }
	inline MooThing *parent() { return(MooThing::lookup(m_parent)); }
	inline MooThing *owner_thing() { return(MooThing::lookup(this->owner())); }
	inline MooThing *contents() { return(m_objects); }
	inline MooThing *next() { return(m_next); }
	inline MooThing *location() { return(m_location); }
	inline int is_a_thing(moo_id_t id);
	inline int is_a_thing(const char *name);
	static inline MooThing *lookup(moo_id_t id) { return(moo_thing_table->get(id)); }

	const char *name();

    protected:
	int assign_id(moo_id_t id);

	int add(MooThing *thing);
	int remove(MooThing *thing);
	int do_action(MooObject *action, MooArgs *args, MooObject **result = NULL);
	MooObject *get_property_raw(const char *name, MooThing **thing);
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

