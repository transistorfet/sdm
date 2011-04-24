/*
 * Object Name:	nil.cpp
 * Description:	Nil Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/objs/nil.h>

MooObjectType moo_nil_obj_type = {
	NULL,
	"nil",
	typeid(MooNil).name(),
	(moo_type_create_t) moo_nil_create
};

MooNil moo_nil;

MooObject *moo_nil_create(void)
{
	return(&moo_nil);
}

MooNil::MooNil()
{
}

MooNil::~MooNil()
{
}

int MooNil::read_entry(const char *type, MooDataFile *data)
{
	return(MOO_HANDLED);
}

int MooNil::write_data(MooDataFile *data)
{
	return(0);
}

int MooNil::to_string(char *buffer, int max)
{
	strncpy(buffer, "nil", max);
	return(strlen(buffer));
}


