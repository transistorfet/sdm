/*
 * Object Name:	object.h
 * Description:	Base Object
 */

#ifndef _SDM_OBJS_OBJECT_H
#define _SDM_OBJS_OBJECT_H

#include <stdarg.h>
#include <sdm/data.h>

#define SDM_NOT_HANDLED		0
#define SDM_HANDLED		1
#define SDM_HANDLED_ALL		2

#define SDM_OBF_RELEASING	0x1000

/** Initialize the preallocade object using the given variable args.  The object will be zero'd before
    the initialization function is called.  If a negative number is returned, the object will destroyed
    and the release function for that object will be called. */
typedef int (*sdm_object_init_t)(struct sdm_object *, int, va_list);
/** Release the resources freed by the object */
typedef void (*sdm_object_release_t)(struct sdm_object *);


typedef class MooObject *(*moo_type_create_t)(void);

typedef struct MooObjectType {
	MooObjectType *parent;
	char *name;
	moo_type_create_t create;
} MooObjectType;

class MooObject {
    protected:
	MooObjectType *m_type;
	int m_bitflags;
    public:
	MooObject();
	virtual ~MooObject();

	int read_file(const char *file, const char *type);
	int write_file(const char *file, const char *type);
	int read_data(MooDataFile *data);

	/** Read an entry from the given open data handle and load the information into the object.  The name of
	    the current entry is given to avoid another call to MooDataFile::read_name().  If an error occurs, a
	    negative number is returned.  If the entry is read successfully, a positive number is returned.  If
	    the entry type is not loadable/recognized by the object, 0 should be returned and the caller shall
	    call the read function of the parent in order to read the entry.  This function should not recursively
	    call the corresponding function of it's parent object */
	virtual int read_entry(const char *name, MooDataFile *data);
	/** Write all data for the object to the given open data handle.  Only data for the immediate object will
	    be written and not data for the object's parent.  The caller shall call the write function for the
	    object's parent before calling this function.  If an error occurs, a negative number is returned. */
	virtual int write_data(MooDataFile *data);

	inline int is_a(MooObjectType *type) {
		MooObjectType *cur;

		for (cur = m_type; cur; cur = cur->parent) {
			if (cur == type)
				return(1);
		}
		return(0);
	}
};

/*
class MooObjectType {
    public:
	const char *name;

	virtual MooObject *create() = 0;
};
*/



int init_object(void);
void release_object(void);

int moo_object_register_type(MooObjectType *);
int moo_object_deregister_type(MooObjectType *);
MooObjectType *moo_object_find_type(const char *, MooObjectType *);

MooObject *create_moo_object(MooObjectType *, int, ...);
void destroy_sdm_object(MooObject *);

#endif

