/*
 * Object Name:	mutable.h
 * Description:	Mutable Object
 */

#ifndef _SDM_OBJS_MUTABLE_H
#define _SDM_OBJS_MUTABLE_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/objs/object.h>

#define MOO_NO_ID		-1
#define MOO_NEW_ID		-2

class MooMutable : public MooObject {
    private:
	moo_id_t m_id;

    public:
	MooMutable();
	MooMutable(moo_id_t id);
	virtual ~MooMutable();

	int is_wizard() { return(m_bitflags & MOO_BF_WIZARD); }
	static int is_wizard(moo_id_t id);

	static MooObject *lookup(moo_id_t id);
	static MooObject *reference(const char *name);
	static MooObject *load_object(moo_id_t id);
	int save_object();
	static int save_all();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int write_object(MooDataFile *data);

	inline moo_id_t id() { return(m_id); }
	int is_assigned(moo_id_t id);

    protected:
	int assign_id(moo_id_t id);
};

int init_mutable(void);
void release_mutable(void);

#endif

