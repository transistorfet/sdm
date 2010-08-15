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
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/actions/action.h>

#define MOO_ACTION_NOT_FOUND	1

#define SDM_NO_ID		-1
#define SDM_NEW_ID		-2

class MooUser;

typedef int moo_id_t;

extern MooArray<MooThing *> *moo_thing_table;

class MooThing : public MooObject {
    protected:
	moo_id_t m_id;
	moo_id_t m_parent;
	MooThing *m_location;
	MooHash<MooObject *> *m_properties;
	MooTree<MooAction *> *m_actions;

	MooThing *m_next;
	MooThing *m_objects;
	MooThing *m_end_objects;

    public:
	MooThing(moo_id_t id = -1, moo_id_t parent = 0);
	virtual ~MooThing();
	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	static MooThing *create(MooUser *user, moo_id_t parent);

	int set_property(const char *name, MooObject *obj);
	int set_property(const char *name, moo_id_t id);
	int set_property(const char *name, double num);
	int set_property(const char *name, const char *str);
	MooObject *get_property(const char *name, MooObjectType *type);

	int set_action(const char *name, MooAction *action);
	MooAction *get_action(const char *name);
	MooAction *get_action_partial(const char *name);
	int do_action(MooAction *action, MooArgs *args);
	int do_action(const char *name, MooArgs *args);
	int do_action(const char *name, MooUser *user, MooThing *object, MooThing *target);

	int moveto(MooThing *thing, MooThing *by);

	inline moo_id_t id() { return(m_id); }
	inline moo_id_t parent() { return(m_parent); }
	inline MooThing *contents() { return(m_objects); }
	inline MooThing *next() { return(m_next); }
	static inline MooThing *lookup(moo_id_t id) { return(moo_thing_table->get(id)); }

	inline int is_a_thing(moo_id_t id);

	MooThing *find(const char *name);
	static MooThing *reference(const char *name);

	int expand_str(char *buffer, int max, MooArgs *args, const char *fmt);
	int expand_reference(char *buffer, int max, MooArgs *args, const char *str, int *used);
	int resolve_reference(char *buffer, int max, MooArgs *args, const char *ref);
	int escape_char(const char *str, char *buffer);
	static int atoi(const char *str, int radix);

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

