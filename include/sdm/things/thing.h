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

class MooThing : public MooObject {
    protected:
	moo_id_t id;
	moo_id_t parent;
	MooThing *location;
	struct sdm_hash *properties;		// MooHash<GameObject> *properties;
	struct sdm_tree *actions;		// MooTree<GameObject> *actions;

	MooThing *next;
	MooThing *objects;
	MooThing *end_objects;
    public:
	MooThing();
	MooThing(moo_id_t id, moo_id_t parent);
	virtual ~MooThing();

	moo_id_t id() { return(this->id); }
	moo_id_t parent() { return(this->parent); }

	int init(/*TODO anything?*/);
	void release();
	virtual int read_entry(const char *type, MooDataFile *);
	virtual int write_data(MooDataFile *);

	int set_property(const char *name, MooObject *obj);
	MooObject *get_property(const char *name, MooObjectType *type);

	int set_action(const char *name, MooAction *action);
	int do_action(const char *name, MooArgs *args);
	int do_abbreved_action(const char *name, MooArgs *args);

	int add(MooThing *thing);
	int remove(MooThing *thing);
};

/*
class MooThingType : public MooObjectType {
    public:
	MooThingType(const char *name) { this->name = name; }
	virtual MooObject *create() { return new MooThing(); }
};

MooThingType moo_thing_type("thing");
*/

#define SDM_THING_ARGS(id, parent)		(id), (parent)

extern MooObjectType moo_thing_obj_type;

int init_thing(void);
void release_thing(void);
MooObject *moo_thing_create(void);

// TODO should these be methods?
int moo_thing_assign_id(MooThing *thing, moo_id_t);
int moo_thing_assign_new_id(MooThing *thing);

/*** Thing ID Functions ***/

// TODO this needs to be changed (possibly class for table management, maybe put moo_thing_is_a as a method of MooThing)

extern int moo_thing_table_size;
extern MooThing **moo_thing_table;

static inline MooThing *moo_thing_lookup_id(moo_id_t id) {
	if ((id >= 0) && (id < moo_thing_table_size))
		return(moo_thing_table[id]);
	return(NULL);
}

static inline int moo_thing_is_a(MooThing *thing, moo_id_t id) {
	for (; thing; thing = moo_thing_lookup_id(thing->parent)) {
		if (thing->id == id)
			return(1);
	}
	return(0);
}

#endif

