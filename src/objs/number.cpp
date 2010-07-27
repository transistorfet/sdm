/*
 * Object Name:	number.c
 * Description:	Number Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/number.h>

struct MooObjectType moo_number_obj_type = {
	NULL,
	"number",
	(moo_type_create_t) moo_number_create
};

int init_moo_number_type(void)
{
	if (moo_object_register_type(&moo_number_obj_type) < 0)
		return(-1);
	return(0);
}

void release_moo_number_type(void)
{
	moo_object_deregister_type(&moo_number_obj_type);
}

MooObject *moo_number_create(void)
{
	return(new MooNumber(0));
}

int MooNumber::read_entry(const char *type, MooDataFile *data)
{
	this->num = data->read_float();
	return(MOO_HANDLED_ALL);
}

int MooNumber::write_data(MooDataFile *data)
{
	data->write_float(this->num);
	return(0);
}

