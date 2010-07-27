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
#include <sdm/objs/object.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/actions/action.h>

#define SDM_NO_ID		-1
#define SDM_NEW_ID		-2

typedef int moo_id_t;

extern MooArray<MooThing *> *moo_thing_table;

class MooThing : public MooObject {
    protected:
	moo_id_t m_id;
	moo_id_t m_parent;
	MooThing *m_location;
	struct sdm_hash *m_properties;		// MooHash<GameObject> *properties;
	struct sdm_tree *m_actions;		// MooTree<GameObject> *actions;

	MooThing *m_next;
	MooThing *m_objects;
	MooThing *m_end_objects;

	int assign_id(moo_id_t id);
    public:
	MooThing();
	MooThing(moo_id_t id, moo_id_t parent);
	virtual ~MooThing();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int set_property(const char *name, MooObject *obj);
	MooObject *get_property(const char *name, MooObjectType *type);

	int set_action(const char *name, MooAction *action);
	int do_action(const char *name, MooArgs *args);
	int do_abbreved_action(const char *name, MooArgs *args);

	int add(MooThing *thing);
	int remove(MooThing *thing);

	inline moo_id_t id() { return(m_id); }
	inline moo_id_t parent() { return(m_parent); }

	static inline MooThing *lookup(moo_id_t id) { return(moo_thing_table->get(id)); }

	inline int is_a_thing(moo_id_t id) {
		for (MooThing *cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
			if (cur->m_id == id)
				return(1);
		}
		return(0);
	}
};

/*
class MooThingType : public MooObjectType {
    public:
	MooThingType(const char *name) { this->name = name; }
	virtual MooObject *create() { return new MooThing(); }
};

MooThingType moo_thing_type("thing");
*/

extern MooObjectType moo_thing_obj_type;

int init_thing(void);
void release_thing(void);
MooObject *moo_thing_create(void);

#endif

