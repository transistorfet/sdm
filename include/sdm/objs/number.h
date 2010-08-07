/*
 * Object Name:	number.h
 * Description:	Number Object
 */

#ifndef _SDM_OBJS_NUMBER_H
#define _SDM_OBJS_NUMBER_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef double moo_number_t;

class MooNumber : public MooObject {
    public:
	moo_number_t num;

	MooNumber(moo_number_t num) { this->num = num; }

	virtual int read_entry(const char *type, MooDataFile *);
	virtual int write_data(MooDataFile *);
};

extern MooObjectType moo_number_obj_type;

MooObject *moo_number_create(void);

#endif

