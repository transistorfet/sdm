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
#define SDM_OBJECT_MAX_INHERITENCE		100

static struct sdm_hash *object_type_list = NULL;

int init_object(void)
{
	if (object_type_list)
		return(1);
	if (!(object_type_list = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, -1, NULL)))
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

int sdm_object_register_type(struct sdm_object_type *type)
{
	return(sdm_hash_add(object_type_list, type->name, type));
}

int sdm_object_deregister_type(struct sdm_object_type *type)
{
	return(sdm_hash_remove(object_type_list, type->name));
}

struct sdm_object_type *sdm_object_find_type(const char *name, struct sdm_object_type *base)
{
	struct sdm_object_type *type, *cur;

	if (!(type = sdm_hash_find(object_type_list, name)))
		return(NULL);
	/** If base is given, then only return this type if it is a subclass of base */
	if (!base)
		return(type);
	for (cur = type; cur; cur = cur->parent) {
		if (cur == base)
			return(type);
	}
	return(NULL);
}


struct sdm_object *create_sdm_object(struct sdm_object_type *type, int nargs, ...)
{
	va_list va;
	struct sdm_object *obj;

	if (!type || !(obj = memory_alloc(type->size)))
		return(NULL);
	memset(obj, '\0', type->size);
	obj->type = type;

	va_start(va, nargs);
	if (type->init && type->init(obj, nargs, va)) {
		destroy_sdm_object(obj);
		return(NULL);
	}
	va_end(va);
	return(obj);
}

void destroy_sdm_object(struct sdm_object *obj)
{
	/** We don't want to attempt to free this object twice */
	if (!obj || (obj->bitflags & SDM_OBF_RELEASING))
		return;
	obj->bitflags |= SDM_OBF_RELEASING;
	if (obj->type->release)
		obj->type->release(obj);
	memory_free(obj);
}

int sdm_object_read_file(struct sdm_object *obj, const char *file, const char *type)
{
	struct sdm_data_file *data;

	sdm_status("Reading %s data from file \"%s\".", type, file);
	if (!(sdm_data_file_exists(file)) || !(data = sdm_data_open(file, SDM_DATA_READ, type)))
		return(-1);
	sdm_object_read_data(obj, data);
	sdm_data_close(data);
	return(0);
}

int sdm_object_write_file(struct sdm_object *obj, const char *file, const char *type)
{
	struct sdm_data_file *data;

	sdm_status("Writing %s data to file \"%s\".", type, file);
	if (!(data = sdm_data_open(file, SDM_DATA_WRITE, type)))
		return(-1);
	sdm_object_write_data(obj, data);
	sdm_data_close(data);
	return(0);
}

int sdm_object_read_data(struct sdm_object *obj, struct sdm_data_file *data)
{
	int res;
	int error = 0;
	const char *type;
	struct sdm_object_type *cur;

	do {
		if (!(type = sdm_data_read_name(data)))
			break;
		for (cur = obj->type; cur; cur = cur->parent) {
			if (cur->read_entry) {
				res = cur->read_entry(obj, type, data);
				if (res < 0)
					error = 1;
				else if (res == SDM_HANDLED)
					break;
				/** We handled the whole rest of the data so just exit */
				else if (res == SDM_HANDLED_ALL)
					return(0);
			}
		}
	} while (sdm_data_read_next(data));
	/** We return if the file loaded incorrectly but we don't stop trying to load the file */
	return(error);
}

int sdm_object_write_data(struct sdm_object *obj, struct sdm_data_file *data)
{
	int i;
	int error = 0;
	struct sdm_object_type *cur;
	struct sdm_object_type *list[SDM_OBJECT_MAX_INHERITENCE];

	for (cur = obj->type, i = 0; cur && (i < SDM_OBJECT_MAX_INHERITENCE); cur = cur->parent, i++)
		list[i] = cur;
	for (i -= 1; i >= 0; i--) {
		if (list[i]->write_data) {
			if (list[i]->write_data(obj, data) < 0)
				error = 1;
		}
	}
	/** We return if there was a problem but we try to write the whole file */
	return(error);
}



