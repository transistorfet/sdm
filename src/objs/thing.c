/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/objs/world.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

#define THING_INIT_PROPERTIES			5
#define THING_INIT_ACTIONS			5

#define THING_TABLE_INIT_SIZE			100
#define THING_TABLE_EXTRA_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define THING_MAX_TABLE_SIZE			65536

struct sdm_object_type sdm_thing_obj_type = {
	NULL,
	"thing",
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
	if (sdm_thing_table)
		return(1);
	if (sdm_object_register_type(&sdm_thing_obj_type) < 0)
		return(-1);
	if (!(sdm_thing_table = (struct sdm_thing **) memory_alloc(sizeof(struct sdm_thing *) * THING_TABLE_INIT_SIZE)))
		return(-1);
	sdm_thing_table_size = THING_TABLE_INIT_SIZE;
	memset(sdm_thing_table, '\0', sizeof(struct sdm_thing *) * sdm_thing_table_size);
	return(0);
}

void release_thing(void)
{
	// TODO free all things in the table???
	if (sdm_thing_table)
		memory_free(sdm_thing_table);
	sdm_object_deregister_type(&sdm_thing_obj_type);
}

int sdm_thing_init(struct sdm_thing *thing, int nargs, va_list va)
{
	if (!(thing->properties = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, THING_INIT_PROPERTIES, (destroy_t) destroy_sdm_object)))
		return(-1);
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(thing->actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, THING_INIT_ACTIONS, (destroy_t) destroy_sdm_object)))
		return(-1);

	/** Set the thing id and add the thing to the table.  If id = SDM_NO_ID, don't add it to a table.
	    If the id = SDM_NEW_ID then assign the next available id */
	if (nargs < 2) {
		thing->id = -1;
		thing->parent = 0;
	}
	else {
		thing->id = va_arg(va, int);
		if (thing->id >= 0)
			sdm_thing_assign_id(thing, thing->id);
		else if (thing->id == SDM_NEW_ID)
			sdm_thing_assign_new_id(thing);
		thing->parent = va_arg(va, sdm_id_t);
	}
	return(0);
}

void sdm_thing_release(struct sdm_thing *thing)
{
	struct sdm_thing *cur, *tmp;

	if (thing->location)
		sdm_thing_remove(thing->location, thing);
	if (thing->properties)
		destroy_sdm_hash(thing->properties);
	if (thing->actions)
		destroy_sdm_hash(thing->actions);
	if ((thing->id >= 0) && (thing->id < sdm_thing_table_size))
		sdm_thing_table[thing->id] = NULL;

	for (cur = thing->objects; cur; cur = tmp) {
		tmp = cur->next;
		destroy_sdm_object(SDM_OBJECT(cur));
	}
}

int sdm_thing_read_entry(struct sdm_thing *thing, const char *type, struct sdm_data_file *data)
{
	int res;
	sdm_id_t id;
	char buffer[STRING_SIZE];
	struct sdm_object *obj = NULL;
	struct sdm_object_type *objtype;

	if (!strcmp(type, "property")) {
		sdm_data_read_attrib(data, "type", buffer, STRING_SIZE);
		if (!(objtype = sdm_object_find_type((*buffer != '\0') ? buffer : "string", NULL)))
			return(-1);
		sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
		if (!(obj = create_sdm_object(objtype, 0)))
			return(-1);
		sdm_data_read_children(data);
		res = sdm_object_read_data(obj, data);
		sdm_data_read_parent(data);
		if ((res < 0) || (sdm_thing_set_property(thing, buffer, obj) < 0)) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "thing")) {
		if (!(obj = create_sdm_object(&sdm_thing_obj_type, 2, SDM_THING_ARGS(SDM_NO_ID, 0))))
			return(-1);
		sdm_data_read_children(data);
		sdm_object_read_data(obj, data);
		sdm_data_read_parent(data);
		sdm_thing_add(thing, SDM_THING(obj));
	}
	else if (!strcmp(type, "action")) {
		sdm_data_read_attrib(data, "type", buffer, STRING_SIZE);
		if (!(objtype = sdm_object_find_type(buffer, &sdm_action_obj_type))
		    || !(obj = create_sdm_object(objtype, 0)))
			return(-1);
		sdm_data_read_attrib(data, "name", buffer, STRING_SIZE);
		sdm_data_read_children(data);
		res = sdm_object_read_data(obj, data);
		sdm_data_read_parent(data);
		if ((res < 0) || (sdm_thing_set_action(thing, buffer, SDM_ACTION(obj)) < 0)) {
			sdm_status("Error loading action, %s.", buffer);
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "id")) {
		id = sdm_data_read_integer_entry(data);
		sdm_thing_assign_id(thing, id);
	}
	else if (!strcmp(type, "location")) {
		id = sdm_data_read_integer_entry(data);
		if ((obj = SDM_OBJECT(sdm_thing_lookup_id(id))))
			sdm_thing_add(SDM_THING(obj), thing);
	}
	else if (!strcmp(type, "parent")) {
		id = sdm_data_read_integer_entry(data);
		thing->parent = id;
	}
	else
		return(SDM_NOT_HANDLED);
	return(SDM_HANDLED);
}

int sdm_thing_write_data(struct sdm_thing *thing, struct sdm_data_file *data)
{
	struct sdm_thing *cur;
	struct sdm_hash_entry *entry;

	sdm_data_write_integer_entry(data, "id", thing->id);
	if (thing->parent >= 0)
		sdm_data_write_integer_entry(data, "parent", thing->parent);
	if (thing->location)
		sdm_data_write_integer_entry(data, "location", SDM_THING(thing->location)->id);

	/** Write the properties to the file */
	sdm_hash_traverse_reset(thing->properties);
	while ((entry = sdm_hash_traverse_next_entry(thing->properties))) {
		sdm_data_write_begin_entry(data, "property");
		sdm_data_write_attrib(data, "type", SDM_OBJECT(entry->data)->type->name);
		sdm_data_write_attrib(data, "name", entry->name);
		sdm_object_write_data(SDM_OBJECT(entry->data), data);
		sdm_data_write_end_entry(data);
	}

	/** Write the actions to the file */
	sdm_hash_traverse_reset(thing->actions);
	while ((entry = sdm_hash_traverse_next_entry(thing->actions))) {
		sdm_data_write_begin_entry(data, "action");
		sdm_data_write_attrib(data, "name", entry->name);
		sdm_data_write_attrib(data, "type", SDM_OBJECT(entry->data)->type->name);
		sdm_object_write_data(SDM_OBJECT(entry->data), data);
		sdm_data_write_end_entry(data);
	}

	/** Write the things we contain to the file */
	for (cur = thing->objects; cur; cur = cur->next) {
		if (sdm_object_is_a(SDM_OBJECT(cur), &sdm_user_obj_type))
			continue;
		else if (sdm_object_is_a(SDM_OBJECT(cur), &sdm_world_obj_type)) {
			sdm_world_write(SDM_WORLD(cur), data);
			/** If we don't continue here, the world will be written to this file which we don't
			    because the world object is a reference only and is written to it's own file */
			continue;
		}
		else
			sdm_data_write_begin_entry(data, "thing");
		sdm_object_write_data(SDM_OBJECT(cur), data);
		sdm_data_write_end_entry(data);
	}
	return(0);
}


int sdm_thing_set_property(struct sdm_thing *thing, const char *name, struct sdm_object *obj)
{
	if (!name || (*name == '\0'))
		return(-1);
	/** If the object is NULL, remove the entry from the table */
	if (!obj)
		return(sdm_hash_remove(thing->properties, name));
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
	/** If the action is NULL, remove the entry from the table */
	if (!action)
		return(sdm_hash_remove(thing->actions, name));
	if (sdm_hash_find(thing->actions, name))
		return(sdm_hash_replace(thing->actions, name, action));
	return(sdm_hash_add(thing->actions, name, action));
}

int sdm_thing_do_action(struct sdm_thing *thing, const char *name, struct sdm_action_args *args)
{
	struct sdm_thing *cur;
	struct sdm_action *action;

	args->result = NULL;
	for (cur = thing; cur; cur = sdm_thing_lookup_id(cur->parent)) {
		if ((action = sdm_hash_find(cur->actions, name)))
			return(action->func(action, thing, args));
	}
	return(1);
}


int sdm_thing_add(struct sdm_thing *thing, struct sdm_thing *obj)
{
	// TODO is this right?  we are passing the obj as the caller because that is how basic_look
	//	expects things but is this correct generally?  if not, we'd just have to make a special
	//	function that redirects the args to look
	/** If the 'on_enter' action returns an error, then the object should not be added */
	if (sdm_thing_do_nil_action(thing, obj, "on_enter") < 0)
		return(-1);
	// TODO we test for location *after* we do on_entre which means we could accidentally call on_enter
	//	multiple times.  It saves us atm for when a new char is registered and you re-add the user
	//	to the same room and that causes the "look" action to be performed
	if (obj->location == thing)
		return(0);
	/** If this object is in another object and it can't be removed, then we don't add it */
	if (obj->location && sdm_thing_remove(obj->location, obj))
		return(-1);
	obj->location = thing;
	obj->next = NULL;
	if (thing->end_objects) {
		thing->end_objects->next = obj;
		thing->end_objects = obj;
	}
	else {
		thing->objects = obj;
		thing->end_objects = obj;
	}
	return(0);
}

int sdm_thing_remove(struct sdm_thing *thing, struct sdm_thing *obj)
{
	struct sdm_thing *cur, *prev;

	// TODO is this correct?
	/** If the 'on_exit' action returns an error, then the object should not be removed */
	if (sdm_thing_do_nil_action(thing, obj, "on_exit") < 0)
		return(-1);
	for (prev = NULL, cur = thing->objects; cur; prev = cur, cur = cur->next) {
		if (cur == obj) {
			if (prev)
				prev->next = cur->next;
			else
				thing->objects = cur->next;
			if (thing->end_objects == cur)
				thing->end_objects = prev;
			cur->location = NULL;
			return(0);
		}
	}
	return(-1);
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
		memset(sdm_thing_table + sdm_thing_table_size, '\0', sizeof(struct sdm_thing *) * (id + THING_TABLE_EXTRA_SIZE - sdm_thing_table_size));
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


