/*
 * Object Name:	nil.cpp
 * Description:	Nil Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/nil.h>

MooObjectType moo_nil_obj_type = {
	NULL,
	"nil",
	typeid(MooNil).name(),
	(moo_type_make_t) make_moo_nil
};

MooNil moo_nil;

MooObject *make_moo_nil(MooDataFile *data)
{
	return(&moo_nil);
}

MooNil::MooNil()
{
	this->set_nofree();
}

MooNil::~MooNil()
{
}

int MooNil::read_entry(const char *type, MooDataFile *data)
{
	return(MooObject::read_entry(type, data));
}

int MooNil::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	return(0);
}

int MooNil::to_string(char *buffer, int max)
{
	strncpy(buffer, "nil", max);
	return(strlen(buffer));
}


