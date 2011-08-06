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

#define MOO_ACTION_NOT_FOUND	1

#define MOO_NO_ID		-1
#define MOO_NEW_ID		-2

#define MOO_TBF_WIZARD		0x01
#define MOO_TBF_WRITING		0x80

extern MooArray<MooThing *> *moo_thing_table;

class MooThing : public MooObject {
    protected:
	int m_bits;
	moo_id_t m_id;
	moo_id_t m_parent;
	MooObjectHash *m_properties;
	MooObjectHash *m_methods;

    public:
	MooThing(moo_id_t id = -1, moo_id_t parent = -1);
	virtual ~MooThing();
	static MooThing *lookup(moo_id_t id);
	MooThing *clone(moo_id_t id = MOO_NEW_ID);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int load();
	virtual int save();
	static int save_all();

	virtual int to_string(char *buffer, int max);
	MooThing *get_thing() { return(this); }

    public:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

    private:
	int set_method(const char *name, MooObject *action);
	MooObject *get_method(const char *name);

    public:
	/// Accessors
	inline moo_id_t id() { return(m_id); }
	inline moo_id_t parent_id() { return(m_parent); }
	inline MooThing *parent() { return(MooThing::lookup(m_parent)); }
	inline MooThing *owner_thing() { return(MooThing::lookup(this->owner())); }
	inline int is_a_thing(moo_id_t id);
	inline int is_a_thing(const char *name);

    public:
	/// Helper Methods
	static MooThing *reference(const char *name);
	int is_wizard() { return(m_bits & MOO_TBF_WIZARD); }
	static int is_wizard(moo_id_t id);

	const char *name();
	int connect(MooTask *task);
	void disconnect();
	int notify(int type, MooThing *thing, MooThing *channel, const char *text);
	static MooThing *get_channel(const char *name);
	void quit();

    protected:
	int assign_id(moo_id_t id);
};

extern MooObjectType moo_thing_obj_type;

int init_thing(void);
void release_thing(void);
MooObject *make_moo_thing(MooDataFile *data);

inline int MooThing::is_a_thing(moo_id_t id)
{
	for (MooThing *cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if (cur->m_id == id)
			return(1);
	}
	return(0);
}

#endif

