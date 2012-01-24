/*
 * Object Name:	boolean.h
 * Description:	Boolean Object
 */

#ifndef _SDM_OBJS_BOOLEAN_H
#define _SDM_OBJS_BOOLEAN_H

#include <stdarg.h>

#include <sdm/objs/object.h>

#define B_FALSE		0
#define B_TRUE		1

class MooBoolean : public MooObject {
	int m_bool;
    public:
	MooBoolean(int value) { m_bool = value; }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	inline int value() { return(m_bool); }

	inline int compare(MooBoolean *b) {
		return(m_bool == b->m_bool);
	}
};

extern MooBoolean moo_true;
extern MooBoolean moo_false;
extern MooObjectType moo_boolean_obj_type;

MooObject *load_moo_boolean(MooDataFile *data);

#endif

