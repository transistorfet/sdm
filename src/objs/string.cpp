/*
 * Object Name:	string.c
 * Description:	String Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/string.h>

MooObjectType moo_string_obj_type = {
	NULL,
	"string",
	typeid(MooString).name(),
	(moo_type_create_t) moo_string_create
};

int init_moo_string_type(void)
{
	if (moo_object_register_type(&moo_string_obj_type) < 0)
		return(-1);
	return(0);
}

void release_moo_string_type(void)
{
	moo_object_deregister_type(&moo_string_obj_type);
}

MooObject *moo_string_create(void)
{
	return(new MooString(""));
}

MooString::MooString(const char *str)
{
	// TODO add valist later
	//char buffer[LARGE_STRING_SIZE];
	//vsnprintf(buffer, LARGE_STRING_SIZE, str, va);
	this->set(str);
}

MooString::~MooString()
{
	memory_free(this->str);
}

int MooString::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

	data->read_string(buffer, STRING_SIZE);
	this->set(buffer);
	return(MOO_HANDLED_ALL);
}

int MooString::write_data(MooDataFile *data)
{
	data->write_string(this->str);
	return(0);
}


int MooString::set(const char *str)
{
	if (this->str)
		memory_free(this->str);
	if (!str) {
		this->str = "";
		return(0);
	}

	this->len = strlen(str);
	if (!(this->str = (char *) memory_alloc(this->len + 1))) {
		this->len = 0;
		return(-1);
	}
	strcpy(this->str, str);
	return(0);
}


