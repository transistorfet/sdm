/*
 * Object Name:	nil.h
 * Description:	Nil Object
 */

#ifndef _SDM_OBJS_NIL_H
#define _SDM_OBJS_NIL_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooNil : public MooObject {
    public:
	MooNil();
	virtual ~MooNil();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual MooThing *get_thing() { return(NULL); }
};

extern MooNil moo_nil;
extern MooObjectType moo_nil_obj_type;

MooObject *make_moo_nil(MooDataFile *data);

#endif

