/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/memory.h>

#include <sdm/objs/object.h>

/** We need this to make a list of object types in sdm_object_write_data() in order to write the data
    to the file with the root object first working down through the children */
#define SDM_OBJECT_MAX_INHERITENCE		100

struct sdm_object *create_sdm_object(struct sdm_object_type *type, ...)
{
	va_list va;
	struct sdm_object *obj;

	if (!(obj = memory_alloc(type->size)))
		return(NULL);
	memset(obj, '\0', type->size);
	obj->type = type;

	va_start(va, type);
	if (type->init && type->init(obj, va)) {
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
	const char *type;
	struct sdm_object_type *cur;

	do {
		if (!(type = sdm_data_read_name(data)))
			break;
		for (cur = obj->type; cur; cur = cur->parent) {
			if (cur->read_entry) {
				res = cur->read_entry(obj, type, data);
				if (res < 0)
					return(-1);
				else if (res > 0)
					break;
			}
		}
	} while (sdm_data_read_next(data));
	return(0);
}

int sdm_object_write_data(struct sdm_object *obj, struct sdm_data_file *data)
{
	int i;
	struct sdm_object_type *cur;
	struct sdm_object_type *list[SDM_OBJECT_MAX_INHERITENCE];

	for (cur = obj->type, i = 0; cur && (i < SDM_OBJECT_MAX_INHERITENCE); cur = cur->parent, i++)
		list[i] = cur;
	for (i -= 1; i >= 0; i--) {
		if (list[i]->write_data)
			list[i]->write_data(obj, data);
	}
	return(0);
}



