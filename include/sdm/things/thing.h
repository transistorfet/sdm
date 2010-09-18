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
#include <sdm/actions/action.h>
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
	// TODO if you made MooObjectHash and MooObjectTree, then you could use their read_entry/write_data functions
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
	int init();
	static MooThing *create(moo_id_t parent);
	MooThing *clone();

	/// Property Methods
	int set_property(const char *name, MooObject *obj);
	int set_property(const char *name, moo_id_t id);
	int set_property(const char *name, long int num);
	int set_property(const char *name, double num);
	int set_property(const char *name, const char *str);
	MooObject *get_property(const char *name, MooObjectType *type);
	MooThing *get_thing_property(const char *name);
	long int get_integer_property(const char *name);
	double get_float_property(const char *name);
	const char *get_string_property(const char *name);

	/// Action Methods
	int set_action(const char *name, MooAction *action);
	MooAction *get_action(const char *name);
	// TODO will we ever use this?  Where would it be used?
	MooAction *get_action_partial(const char *name);
	// TODO should these take user, or should it assume it based on the current task??
	// TODO should these take a channel parameter?
	int do_action(MooThing *user, MooThing *channel, const char *text, MooObject **result = NULL);
	int do_action(MooThing *user, MooThing *channel, const char *name, const char *text, MooObject **result = NULL);
	int do_action(const char *name, MooArgs *args, MooObject **result = NULL);
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
	static inline MooThing *lookup(moo_id_t id) { return(moo_thing_table->get(id)); }

	const char *name();

	/// String Parsers
	static int expand_str(char *buffer, int max, MooArgs *args, const char *fmt);
	static int expand_reference(char *buffer, int max, MooArgs *args, const char *str, int *used);
	static int resolve_reference(char *buffer, int max, MooArgs *args, const char *ref);
	static int escape_char(const char *str, char *buffer);

    protected:
	int assign_id(moo_id_t id);

	int add(MooThing *thing);
	int remove(MooThing *thing);
	int do_action(MooAction *action, MooArgs *args, MooObject **result = NULL);
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

