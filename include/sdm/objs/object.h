/*
 * Object Name:	object.h
 * Description:	Base Object
 */

#ifndef _SDM_OBJS_OBJECT_H
#define _SDM_OBJS_OBJECT_H

#include <typeinfo>
#include <stdarg.h>
#include <sdm/gc.h>
#include <sdm/data.h>

#define MOO_NOT_HANDLED		0
#define MOO_HANDLED		1
#define MOO_HANDLED_ALL		2

#define MOO_BF_WIZARD		0x01
#define MOO_BF_DELETING		0x10
#define MOO_BF_WRITING		0x20

class MooDataFile;

typedef class MooObject *(*moo_type_load_t)(MooDataFile *data);

typedef struct MooObjectType {
	const char *m_name;
	const char *m_realname;
	moo_type_load_t m_load;
} MooObjectType;

class MooThing;
class MooCodeFrame;
class MooObjectHash;
class MooObjectArray;

class MooObject : public MooGC {
    protected:
	int m_bitflags;

    public:
	MooObject();
	virtual ~MooObject() { }
	virtual void print_debug();

	const MooObjectType *objtype();
	const char *objtype_name() { return(this->objtype()->m_name); }
	int is_true();

	int read_data(MooDataFile *data);
	static MooObject *read_object(MooDataFile *data, const char *type);

	/** Read an entry from the given open data handle and load the information into the object.  The name of
	    the current entry is given to avoid another call to MooDataFile::read_name().  If an error occurs, a
	    negative number is returned.  If the entry is read successfully, a positive number is returned.  If
	    the entry type is not loadable/recognized by the object, 0 should be returned and the caller shall
	    call the read function of the parent in order to read the entry.  This function should not recursively
	    call the corresponding function of it's parent object */
	virtual int read_entry(const char *type, MooDataFile *data) { return(MOO_HANDLED); }
	/** Write all data for the object to the given open data handle.  Only data for the immediate object will
	    be written and not data for the object's parent.  The caller shall call the write function for the
	    object's parent before calling this function.  If an error occurs, a negative number is returned. */
	virtual int write_data(MooDataFile *data) { return(0); }

	virtual int to_string(char *buffer, int max) { buffer[0] = '\0'; return(0); }

	virtual long int get_integer() { throw MooException("Unable to convert to integer"); }
	virtual double get_float() { throw MooException("Unable to convert to float"); }
	virtual const char *get_string() { throw MooException("Unable to convert to string"); }
	virtual MooThing *get_thing() { throw MooException("Unable to convert to thing"); }

	static MooObject *resolve(const char *name, MooObjectHash *env, MooObject *value = NULL, MooObject **parent = NULL);
	MooObject *resolve_property(const char *name, MooObject *value = NULL);
	MooObject *resolve_method(const char *name, MooObject *value = NULL);

	int call_method(MooObject *channel, const char *name, MooObject **result = NULL, MooObject *obj1 = NULL, MooObject *obj2 = NULL, MooObject *obj3 = NULL, MooObject *obj4 = NULL, MooObject *obj5 = NULL);
	int evaluate(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args);

	static int format(char *buffer, int max, MooObjectHash *env, const char *fmt);
	static int expand_reference(char *buffer, int max, MooObjectHash *env, const char *str, int *used);
	static int escape_char(const char *str, char *buffer);

    private:
	/// Object Member Access Functions
	virtual MooObject *access_property(const char *name, MooObject *value = NULL) { throw moo_type_error; }
	virtual MooObject *access_method(const char *name, MooObject *value = NULL) { throw moo_type_error; }
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args) { throw moo_evaluate_error; }
	friend class MooTask;

    protected:
	int is_deleting() { return((m_bitflags & MOO_BF_DELETING) ? 1 : 0); }
	void set_delete() { m_bitflags |= MOO_BF_DELETING; }
};

int init_object(void);
void release_object(void);

int moo_object_register_type(const MooObjectType *type);
int moo_object_deregister_type(const MooObjectType *type);
const MooObjectType *moo_object_find_type(const char *name);

#endif

