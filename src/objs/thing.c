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
#include <sdm/modules/module.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

#define THING_TABLE_INIT_SIZE			100
#define THING_TABLE_EXTRA_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define THING_MAX_TABLE_SIZE			65536

struct sdm_action {
	sdm_action_t func;
	void *ptr;
	destroy_t destroy;
};

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

static void sdm_thing_destroy_action(struct sdm_action *);

int init_thing(void)
{
	if (!(sdm_thing_table = (struct sdm_thing **) memory_alloc(sizeof(struct sdm_thing *) * THING_TABLE_INIT_SIZE)))
		return(-1);
	sdm_thing_table_size = THING_TABLE_INIT_SIZE;
	return(0);
}

int release_thing(void)
{
	// TODO free all things??
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
	if (!(thing->actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, (destroy_t) sdm_thing_destroy_action)))
		return(-1);

	/** Set the thing id and add the thing to the table.  If id = 0, don't add it to a table.
	    If the id = -1 then assign the next available id */
	thing->id = va_arg(va, int);
	if (thing->id > 0)
		sdm_thing_assign_id(thing, thing->id);
	else if (thing->id == -1)
		sdm_thing_assign_new_id(thing);

	thing->parent = va_arg(va, sdm_id_t);
	return(0);
}

void sdm_thing_release(struct sdm_thing *thing)
{
	if (thing->owner)
		sdm_container_remove(thing->owner, thing);
	if (thing->properties)
		destroy_sdm_hash(thing->properties);
	if (thing->actions)
		destroy_sdm_hash(thing->actions);
	if ((thing->id > 0) && (thing->id < sdm_thing_table_size))
		sdm_thing_table[thing->id] = NULL;
}

int sdm_thing_read_entry(struct sdm_thing *thing, const char *type, struct sdm_data_file *data)
{
	sdm_id_t id;
	sdm_number_t num;
	struct sdm_object *obj;
	char buffer[STRING_SIZE];

	if (!strcmp(type, "string")) {
		if ((sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		    || !(obj = create_sdm_object(&sdm_string_obj_type, buffer)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0) {
			destroy_sdm_object(obj);
			return(-1);
		}
		sdm_thing_set_property(thing, buffer, obj);
	}
	else if (!strcmp(type, "number")) {
		if (((num = sdm_data_read_float(data)) < 0)
		    || !(obj = create_sdm_object(&sdm_number_obj_type, num)))
			return(-1);
		if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0) {
			destroy_sdm_object(obj);
			return(-1);
		}
		sdm_thing_set_property(thing, buffer, obj);
	}
	else if (!strcmp(type, "action")) {
		if (sdm_data_read_attrib(data, "type", buffer, STRING_SIZE) < 0)
			return(-1);
		sdm_module_read_action(buffer, thing, data);
	}
	else if (!strcmp(type, "id")) {
		if ((id = sdm_data_read_integer(data)) > 0)
			sdm_thing_assign_id(thing, id);
	}
	else if (!strcmp(type, "owner")) {
		// TODO this will not load add the object to the owner if the owner hasn't been loaded yet =/
		if (((id = sdm_data_read_integer(data)) > 0) && (obj = SDM_OBJECT(sdm_thing_lookup_id(id))))
			sdm_container_add(SDM_CONTAINER(obj), thing);
	}
	else if (!strcmp(type, "parent")) {
		if ((id = sdm_data_read_integer(data)) > 0)
			thing->parent = id;
	}
	else {
		return(1);
	}
	return(0);
}

int sdm_thing_write_data(struct sdm_thing *thing, struct sdm_data_file *data)
{
	struct sdm_hash_entry *entry;

	sdm_data_write_integer_entry(data, "id", thing->id);
	if (thing->parent > 0)
		sdm_data_write_integer_entry(data, "parent", thing->parent);
	if (thing->owner)
		sdm_data_write_integer_entry(data, "owner", SDM_THING(thing->owner)->id);
	/** Write the properties to the file */
	sdm_hash_traverse_reset(thing->properties);
	while ((entry = sdm_hash_traverse_next_entry(thing->properties))) {
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
		// TODO write actions
	}
	return(0);
}


int sdm_thing_set_property(struct sdm_thing *thing, const char *name, struct sdm_object *obj)
{
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


int sdm_thing_set_action(struct sdm_thing *thing, const char *name, sdm_action_t func, void *ptr, destroy_t destroy)
{
	int res;
	struct sdm_action *action;

	if (!(action = (struct sdm_action *) memory_alloc(sizeof(struct sdm_action))))
		return(-1);
	action->func = func;
	action->ptr = ptr;
	action->destroy = destroy;
	if (sdm_hash_find(thing->actions, name))
		res = sdm_hash_replace(thing->actions, name, action);
	else
		res = sdm_hash_add(thing->actions, name, action);

	if (res >= 0)
		return(0);
	memory_free(action);
	return(-1);
}

int sdm_thing_do_action(struct sdm_thing *thing, struct sdm_user *user, const char *name, const char *args)
{
	struct sdm_thing *cur;
	struct sdm_action *action;

	for (cur = thing; cur; cur = sdm_thing_lookup_id(cur->parent)) {
		if ((action = sdm_hash_find(cur->actions, name))) {
			action->func(action->ptr, user, thing, args);
			return(0);
		}
	}
	return(1);
}


int sdm_thing_assign_id(struct sdm_thing *thing, sdm_id_t id)
{
	struct sdm_thing **tmp;

	/** If the thing already has a valid id, remove it's existing entry in the table.  If an error
	    happens in this function, the thing's id will be 0 in case the caller doesn't check */
	if ((thing->id > 0) && (thing->id < sdm_thing_table_size)) {
		sdm_thing_table[thing->id] = NULL;
		thing->id = 0;
	}

	/** If the new id is invalid, then don't do anything.  This maximum size is to prevent us from
	    inadvertanly creating a gigantic table */
	if ((id <= 0) || (id >= THING_MAX_TABLE_SIZE))
		return(-1);

	/** Increase the size of the table if id is too big to have a spot in the table */
	if (id >= sdm_thing_table_size) {
		if (!(tmp = (struct sdm_thing **) memory_realloc(sdm_thing_table, sizeof(struct sdm_thing *) * (id + THING_TABLE_EXTRA_SIZE))))
			return(-1);
		sdm_thing_table = tmp;
		sdm_thing_table_size = id + THING_TABLE_EXTRA_SIZE;
	}

	// TODO should we destroy the object or prevent the new object from taking that ID?
	/** If there is already a thing with the same id, then destroy it */
	if (sdm_thing_table[id])
		destroy_sdm_object(SDM_OBJECT(sdm_thing_table[id]));
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


/*** Local Functions ***/

static void sdm_thing_destroy_action(struct sdm_action *action)
{
	if (action->destroy)
		action->destroy(action->ptr);
	memory_free(action);
}


