/*
 * Object Name:	object.h
 * Description:	Base Object
 */

#ifndef _SDM_OBJS_OBJECT_H
#define _SDM_OBJS_OBJECT_H

#include <stdarg.h>
#include <sdm/data.h>

#define MOO_NOT_HANDLED		0
#define MOO_HANDLED		1
#define MOO_HANDLED_ALL		2

#define SDM_OBF_RELEASING	0x1000

typedef class MooObject *(*moo_type_create_t)(void);

typedef struct MooObjectType {
	MooObjectType *m_parent;
	char *m_name;
	moo_type_create_t m_create;
} MooObjectType;

class MooObject {
    protected:
	MooObjectType *m_type;
	int m_bitflags;
    public:
	MooObject();
	virtual ~MooObject();

	MooObjectType *type() { return(m_type); }
	const char *type_name() { return(m_type->m_name); }

	int read_file(const char *file, const char *type);
	int write_file(const char *file, const char *type);
	int read_data(MooDataFile *data);

	/** Read an entry from the given open data handle and load the information into the object.  The name of
	    the current entry is given to avoid another call to MooDataFile::read_name().  If an error occurs, a
	    negative number is returned.  If the entry is read successfully, a positive number is returned.  If
	    the entry type is not loadable/recognized by the object, 0 should be returned and the caller shall
	    call the read function of the parent in order to read the entry.  This function should not recursively
	    call the corresponding function of it's parent object */
	virtual int read_entry(const char *name, MooDataFile *data) = 0;
	/** Write all data for the object to the given open data handle.  Only data for the immediate object will
	    be written and not data for the object's parent.  The caller shall call the write function for the
	    object's parent before calling this function.  If an error occurs, a negative number is returned. */
	virtual int write_data(MooDataFile *data) = 0;

	inline int is_a(MooObjectType *type) {
		MooObjectType *cur;

		for (cur = m_type; cur; cur = cur->m_parent) {
			if (cur == type)
				return(1);
		}
		return(0);
	}
};

int init_object(void);
void release_object(void);

int moo_object_register_type(MooObjectType *type);
int moo_object_deregister_type(MooObjectType *type);
MooObjectType *moo_object_find_type(const char *name, MooObjectType *base);

MooObject *moo_make_object(MooObjectType *type);

#endif

