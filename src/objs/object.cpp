/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>

#include <sdm/objs/object.h>

/** We need this to make a list of object types in sdm_object_write_data() in order to write the data
    to the file with the root object first working down through the children */
#define MOO_OBJECT_MAX_INHERITENCE		100

static struct sdm_hash *object_type_list = NULL;

int init_object(void)
{
	if (object_type_list)
		return(1);
	if (!(object_type_list = create_sdm_hash(0, -1, NULL)))
		return(-1);
	return(0);
}

void release_object(void)
{
	if (!object_type_list)
		return;
	destroy_sdm_hash(object_type_list);
	object_type_list = NULL;
}

int moo_object_register_type(MooObjectType *type)
{
	return(sdm_hash_add(object_type_list, type->m_name, type));
}

int moo_object_deregister_type(MooObjectType *type)
{
	return(sdm_hash_remove(object_type_list, type->m_name));
}

MooObjectType *moo_object_find_type(const char *name, MooObjectType *base)
{
	MooObjectType *type, *cur;

	if (!(type = (MooObjectType *) sdm_hash_find(object_type_list, name)))
		return(NULL);
	/** If base is given, then only return this type if it is a subclass of base */
	if (!base)
		return(type);
	for (cur = type; cur; cur = cur->m_parent) {
		if (cur == base)
			return(type);
	}
	return(NULL);
}


MooObject *moo_make_object(MooObjectType *type)
{
	if (!type)
		return(NULL);
	try {
		return(type->m_create());
	} catch(int e) {
		return(NULL);
	}
}

int MooObject::read_file(const char *file, const char *type)
{
	int res;
	MooDataFile *data;

	sdm_status("Reading %s data from file \"%s\".", type, file);
	if (!(moo_data_file_exists(file)))
		return(-1);
	data = new MooDataFile(file, MOO_DATA_READ, type);
	res = this->read_data(data);
	delete data;
	return(res);
}

int MooObject::write_file(const char *file, const char *type)
{
	MooDataFile *data;

	sdm_status("Writing %s data to file \"%s\".", type, file);
	data = new MooDataFile(file, MOO_DATA_WRITE, type);
	this->write_data(data);
	delete data;
	return(0);
}

int MooObject::read_data(MooDataFile *data)
{
	int res;
	int error = 0;
	const char *type;

	do {
		if (!(type = data->read_name()))
			break;
		this->read_entry(type, data);
		if (res < 0)
			error = 1;
		/** We handled the whole rest of the data so just exit */
		else if (res == MOO_HANDLED_ALL)
			return(error);
	} while (data->read_next());
	/** We return if the file loaded incorrectly but we don't stop trying to load the file */
	return(error);
}


