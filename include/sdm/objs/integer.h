/*
 * Object Name:	integer.h
 * Description:	Integer Object
 */

#ifndef _SDM_OBJS_INTEGER_H
#define _SDM_OBJS_INTEGER_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef long int moo_integer_t;

class MooInteger : public MooObject {
    public:
	moo_integer_t m_num;

	MooInteger(moo_integer_t num) { m_num = num; }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int parse_arg(MooThing *user, MooThing *channel, char *text);
	virtual int to_string(char *buffer, int max);

	virtual long int get_integer() { return(m_num); }
};

extern MooObjectType moo_integer_obj_type;

MooObject *moo_integer_create(void);

#endif

