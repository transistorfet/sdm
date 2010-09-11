/*
 * Object Name:	float.h
 * Description:	Float Object
 */

#ifndef _SDM_OBJS_FLOAT_H
#define _SDM_OBJS_FLOAT_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef double moo_float_t;

class MooFloat : public MooObject {
    public:
	moo_float_t m_num;

	MooFloat(moo_float_t num) { m_num = num; }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int parse_arg(MooThing *user, MooThing *channel, char *text);
	virtual int to_string(char *buffer, int max);
};

extern MooObjectType moo_float_obj_type;

MooObject *moo_float_create(void);

#endif
