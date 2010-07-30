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

static MooHash<MooObjectType *> *object_type_list = NULL;

int init_object(void)
{
	if (object_type_list)
		return(1);
	object_type_list = new MooHash<MooObjectType *>(MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	return(0);
}

void release_object(void)
{
	if (!object_type_list)
		return;
	delete object_type_list;
	object_type_list = NULL;
}

int moo_object_register_type(MooObjectType *type)
{
	return(object_type_list->set(type->m_name, type));
}

int moo_object_deregister_type(MooObjectType *type)
{
	return(object_type_list->remove(type->m_name));
}

MooObjectType *moo_object_find_type(const char *name, MooObjectType *base)
{
	MooObjectType *type, *cur;

	if (!(type = object_type_list->get(name)))
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


