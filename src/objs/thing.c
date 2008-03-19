/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/container.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

#define THING_TABLE_INIT_SIZE			100
#define THING_TABLE_EXTRA_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define THING_MAX_TABLE_SIZE			65536

struct sdm_object_type sdm_thing_obj_type = {
	NULL,
	sizeof(struct sdm_thing),
	NULL,
	(sdm_object_init_t) sdm_thing_init,
	(sdm_object_release_t) sdm_thing_release,
	(sdm_object_read_entry_t) sdm_thing_read_entry,
	(sdm_object_write_data_t) sdm_thing_write_data
};

static sdm_id_t next_id = 1;
int sdm_thing_table_size = 0;
struct sdm_thing **sdm_thing_table = NULL;

int init_thing(void)
{
	if (!(sdm_thing_table = (struct sdm_thing **) memory_alloc(sizeof(struct sdm_thing *) * THING_TABLE_INIT_SIZE)))
		return(-1);
	sdm_thing_table_size = THING_TABLE_INIT_SIZE;
	return(0);
}

int release_thing(void)
{
	// TODO free all things in the table???
	memory_free(sdm_thing_table);
	return(0);
}

int sdm_thing_init(struct sdm_thing *thing, va_list va)
{
	if (!(thing->properties = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, (destroy_t) destroy_sdm_object)))
		return(-1);
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(thing->actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, (destroy_t) destroy_sdm_object)))
		return(-1);

	/** Set the thing id and add the thing to the table.  If id = SDM_NO_ID, don't add it to a table.
	    If the id = SDM_NEW_ID then assign the next available id */
	thing->id = va_arg(va, int);
	if (thing->id >= 0)
		sdm_thing_assign_id(thing, thing->id);
	else if (thing->id == SDM_NEW_ID)
		sdm_thing_assign_new_id(thing);

	thing->parent = va_arg(va, sdm_id_t);
	return(0);
}

void sdm_thing_release(struct sdm_thing *thing)
{
	if (thing->location)
		sdm_container_remove(thing->location, thing);
	if (thing->properties)
		destroy_sdm_hash(thing->properties);
	if (thing->actions)
		destroy_sdm_hash(thing->actions);
	if ((thing->id >= 0) && (thing->id < sdm_thing_table_size))
		sdm_thing_table[thing->id] = NULL;
}

int sdm_thing_read_entry(struct sdm_thing *thing, const char *type, struct sdm_data_file *data)
{
	sdm_id_t id;
	sdm_number_t num;
	struct sdm_object *obj;
	char buffer[STRING_SIZE];
	struct sdm_object_type *objtype;

	if (!strcmp(type, "string")) {
		sdm_data_read_string(data, buffer, STRING_SIZE);
		if (!(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
		if (sdm_thing_set_property(thing, buffer, obj) < 0) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "number")) {
		num = sdm_data_read_float(data);
		if (!(obj = create_sdm_object(&sdm_number_obj_type, num)))
			return(-1);
		sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
		if (sdm_thing_set_property(thing, buffer, obj)) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "action")) {
		sdm_data_read_attrib(data, "type", buffer, STRING_SIZE);
		if (!(objtype = sdm_object_find_type(buffer, &sdm_action_obj_type))
		    || !(obj = create_sdm_object(objtype)))
			return(-1);
		sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
		if ((obj->type->read_entry && (obj->type->read_entry(obj, type, data) != SDM_HANDLED))
		    || (sdm_thing_set_action(thing, buffer, SDM_ACTION(obj)) < 0)) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "id")) {
		id = sdm_data_read_integer(data);
		sdm_thing_assign_id(thing, id);
	}
	else if (!strcmp(type, "location")) {
		id = sdm_data_read_integer(data);
		if ((obj = SDM_OBJECT(sdm_thing_lookup_id(id))))
			sdm_container_add(SDM_CONTAINER(obj), thing);
	}
	else if (!strcmp(type, "parent")) {
		id = sdm_data_read_integer(data);
		thing->parent = id;
	}
	else
		return(SDM_NOT_HANDLED);
	return(SDM_HANDLED);
}

int sdm_thing_write_data(struct sdm_thing *thing, struct sdm_data_file *data)
{
	struct sdm_hash_entry *entry;

	sdm_data_write_integer_entry(data, "id", thing->id);
	if (thing->parent >= 0)
		sdm_data_write_integer_entry(data, "parent", thing->parent);
	if (thing->location)
		sdm_data_write_integer_entry(data, "location", SDM_THING(thing->location)->id);
	/** Write the properties to the file */
	sdm_hash_traverse_reset(thing->properties);
	while ((entry = sdm_hash_traverse_next_entry(thing->properties))) {
		// TODO what do we do about other property object types?
		// TODO if you can get a typename from the object's type, you can make this like the action
		// 	writing code
		if (SDM_OBJECT(entry->data)->type == &sdm_string_obj_type) {
			sdm_data_write_begin_entry(data, "string");
			sdm_data_write_attrib(data, "name", entry->name);
			sdm_data_write_string(data, SDM_STRING(SDM_OBJECT(entry->data))->str);
			sdm_data_write_end_entry(data);
		}
		else if (SDM_OBJECT(entry->data)->type == &sdm_number_obj_type) {
			sdm_data_write_begin_entry(data, "number");
			sdm_data_write_attrib(data, "name", entry->name);
			sdm_data_write_float(data, SDM_NUMBER(SDM_OBJECT(entry->data))->num);
			sdm_data_write_end_entry(data);
		}
	}
	/** Write the actions to the file */
	sdm_hash_traverse_reset(thing->actions);
	while ((entry = sdm_hash_traverse_next_entry(thing->actions))) {
		sdm_data_write_begin_entry(data, "action");
		sdm_data_write_attrib(data, "name", entry->name);
		// TODO can you somehow get the type name
		if  (SDM_OBJECT(entry->data)->type->write_data)
			SDM_OBJECT(entry->data)->type->write_data(SDM_OBJECT(entry->data), data);
		sdm_data_write_end_entry(data);
	}
	return(0);
}


int sdm_thing_set_property(struct sdm_thing *thing, const char *name, struct sdm_object *obj)
{
	if (!name || (*name == '\0'))
		return(-1);
	if (sdm_hash_find(thing->properties, name))
		return(sdm_hash_replace(thing->properties, name, obj));
	return(sdm_hash_add(thing->properties, name, obj));
}

struct sdm_object *sdm_thing_get_property(struct sdm_thing *thing, const char *name, struct sdm_object_type *type)
{
	struct sdm_thing *cur;
	struct sdm_object *obj;

	for (cur = thing; cur; cur = sdm_thing_lookup_id(cur->parent)) {
		if (!(obj = sdm_hash_find(thing->properties, name)))
			continue;
		if (!type || sdm_object_is_a(obj, type))
			return(obj);
	}
	return(NULL);
}


int sdm_thing_set_action(struct sdm_thing *thing, const char *name, struct sdm_action *action)
{
	if (!name || (*name == '\0'))
		return(-1);
	if (sdm_hash_find(thing->actions, name))
		return(sdm_hash_replace(thing->actions, name, action));
	return(sdm_hash_add(thing->actions, name, action));
}

int sdm_thing_do_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *name, struct sdm_thing *target, const char *args)
{
	struct sdm_thing *cur;
	struct sdm_action *action;

	for (cur = thing; cur; cur = sdm_thing_lookup_id(cur->parent)) {
		if ((action = sdm_hash_find(cur->actions, name))) {
			action->func(action, caller, thing, target, args);
			return(0);
		}
	}
	return(1);
}


int sdm_thing_assign_id(struct sdm_thing *thing, sdm_id_t id)
{
	struct sdm_thing **tmp;

	/** If the thing already has a valid id, remove it's existing entry in the table.  If an error
	    happens in this function, the thing's id will be -1 in case the caller doesn't check */
	if ((thing->id > 0) && (thing->id < sdm_thing_table_size)) {
		sdm_thing_table[thing->id] = NULL;
		thing->id = -1;
	}

	/** If the new id is invalid, then don't do anything.  This maximum size is to prevent us from
	    inadvertanly creating a gigantic table */
	if ((id < 0) || (id >= THING_MAX_TABLE_SIZE))
		return(-1);

	/** Increase the size of the table if id is too big to have a spot in the table */
	if (id >= sdm_thing_table_size) {
		if (!(tmp = (struct sdm_thing **) memory_realloc(sdm_thing_table, sizeof(struct sdm_thing *) * (id + THING_TABLE_EXTRA_SIZE))))
			return(-1);
		sdm_thing_table = tmp;
		sdm_thing_table_size = id + THING_TABLE_EXTRA_SIZE;
	}

	// TODO should we destroy the object or prevent the new object from taking that ID?
	//if (sdm_thing_table[id])
	//	destroy_sdm_object(SDM_OBJECT(sdm_thing_table[id]));
	/** If there is already a thing with the same id, then don't make the assignmet */
	if (sdm_thing_table[id])
		return(-1);
	thing->id = id;
	sdm_thing_table[id] = thing;

	// TODO for now we always make the next id the largest unassign id
	if (next_id <= id)
		next_id = id + 1;
	return(0);
}

int sdm_thing_assign_new_id(struct sdm_thing *thing)
{
	// TODO for now we will not assign the ids of holes in the table
	/** Keep in mind that as it stands, the id of the parent of an object must be smaller than the id
	    of that object itself or the loading of the object will fail. */
	return(sdm_thing_assign_id(thing, next_id++));
}


