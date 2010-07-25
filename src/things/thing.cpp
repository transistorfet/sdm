/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/tree.h>
#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

#define THING_INIT_PROPERTIES			5
#define THING_INIT_ACTIONS			5

#define THING_TABLE_INIT_SIZE			100
#define THING_TABLE_EXTRA_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define THING_MAX_TABLE_SIZE			65536

MooObjectType moo_thing_obj_type = {
	NULL,
	"thing",
	(moo_type_create_t) moo_thing_create
};

static moo_id_t next_id = 1;
int moo_thing_table_size = 0;
//MooThing **moo_thing_table = NULL;
MooArray<MooThing> *moo_thing_table = NULL;

int init_thing(void)
{
	if (moo_thing_table)
		return(1);
	if (moo_object_register_type(&moo_thing_obj_type) < 0)
		return(-1);
	// TODO convert this to new/delet
	if (!(moo_thing_table = (MooThing **) memory_alloc(sizeof(MooThing *) * THING_TABLE_INIT_SIZE)))
		return(-1);
	moo_thing_table_size = THING_TABLE_INIT_SIZE;
	memset(moo_thing_table, '\0', sizeof(MooThing *) * moo_thing_table_size);
	return(0);
}

void release_thing(void)
{
	// TODO free all things in the table???
	if (moo_thing_table)
		memory_free(moo_thing_table);
	sdm_object_deregister_type(&moo_thing_obj_type);
}

MooObject *moo_thing_create(void)
{
	return(new MooThing());
}

MooThing::MooThing()
{
	MooThing::MooThing(-1, 0);
}

MooThing::MooThing(moo_id_t id, moo_id_t parent)
{
	if (!(this->properties = create_sdm_hash(0, THING_INIT_PROPERTIES, (destroy_t) destroy_sdm_object)))
		throw -1;
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(this->actions = create_sdm_tree(0, (destroy_t) destroy_sdm_object)))
		throw -1;

	/** Set the thing id and add the thing to the table.  If id = SDM_NO_ID, don't add it to a table.
	    If the id = SDM_NEW_ID then assign the next available id */
	this->id = id;
	if (this->id >= 0)
		moo_thing_assign_id(this, this->id);
	else if (this->id == SDM_NEW_ID)
		moo_thing_assign_new_id(this);
	this->parent = parent;
	return(0);
}

MooThing::~MooThing()
{
	MooThing *cur, *tmp;

	if (this->location)
		this->location->remove(this);
	if (this->properties)
		destroy_sdm_hash(this->properties);
	if (this->actions)
		destroy_sdm_tree(this->actions);
	if ((this->id >= 0) && (this->id < moo_thing_table_size))
		moo_thing_table[this->id] = NULL;

	for (cur = this->objects; cur; cur = tmp) {
		tmp = cur->next;
		delete cur;
	}
}

int MooThing::read_entry(const char *type, MooDataFile *data)
{
	int res;
	moo_id_t id;
	char buffer[STRING_SIZE];
	MooObject *obj = NULL;
	MooObjectType *objtype;

	if (!strcmp(type, "property")) {
		data->read_attrib("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type((*buffer != '\0') ? buffer : "string", NULL)))
			return(-1);
		data->read_attrib("name", buffer, STRING_SIZE);
		if (!(obj = create_sdm_object(objtype, 0)))
			return(-1);
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (sdm_thing_set_property(thing, buffer, obj) < 0)) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else if (!strcmp(type, "thing")) {
		if (!(obj = create_sdm_object(&moo_thing_obj_type, 2, SDM_THING_ARGS(SDM_NO_ID, 0))))
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
		moo_thing_assign_id(thing, id);
	}
	else if (!strcmp(type, "location")) {
		id = sdm_data_read_integer_entry(data);
		if ((obj = SDM_OBJECT(moo_thing_lookup_id(id))))
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
	struct sdm_hash_entry *hentry;
	struct sdm_tree_entry *tentry;

	sdm_data_write_integer_entry(data, "id", thing->id);
	if (thing->parent >= 0)
		sdm_data_write_integer_entry(data, "parent", thing->parent);
	if (thing->location)
		sdm_data_write_integer_entry(data, "location", SDM_THING(thing->location)->id);

	/** Write the properties to the file */
	sdm_hash_traverse_reset(thing->properties);
	while ((hentry = sdm_hash_traverse_next_entry(thing->properties))) {
		sdm_data_write_begin_entry(data, "property");
		sdm_data_write_attrib(data, "type", SDM_OBJECT(hentry->data)->type->name);
		sdm_data_write_attrib(data, "name", hentry->name);
		sdm_object_write_data(SDM_OBJECT(hentry->data), data);
		sdm_data_write_end_entry(data);
	}

	/** Write the actions to the file */
	sdm_tree_traverse_reset(thing->actions);
	while ((tentry = sdm_tree_traverse_next_entry(thing->actions))) {
		sdm_data_write_begin_entry(data, "action");
		sdm_data_write_attrib(data, "type", SDM_OBJECT(tentry->data)->type->name);
		sdm_data_write_attrib(data, "name", tentry->name);
		sdm_object_write_data(SDM_OBJECT(tentry->data), data);
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


int MooThing::set_property(const char *name, MooObject *obj)
{
	if (!name || (*name == '\0'))
		return(-1);
	/** If the object is NULL, remove the entry from the table */
	if (!obj)
		return(sdm_hash_remove(this->properties, name));
	if (sdm_hash_find(this->properties, name))
		return(sdm_hash_replace(this->properties, name, obj));
	return(sdm_hash_add(this->properties, name, obj));
}

MooObject *MooThing::get_property(const char *name, MooObjectType *type)
{
	MooThing *cur;
	MooObject *obj;

	for (cur = this; cur; cur = moo_thing_lookup_id(cur->parent)) {
		if (!(obj = sdm_hash_find(this->properties, name)))
			continue;
		if (!type || sdm_object_is_a(obj, type))
			return(obj);
	}
	return(NULL);
}


int MooThing::set_action(const char *name, MooAction *action)
{
	if (!name || (*name == '\0'))
		return(-1);
	/** If the action is NULL, remove the entry from the table */
	if (!action)
		return(sdm_tree_remove(this->actions, name));
	if (sdm_tree_find(this->actions, name))
		return(sdm_tree_replace(this->actions, name, action));
	return(sdm_tree_add(this->actions, name, action));
}

int MooThing::do_action(const char *name, MooArgs *args)
{
	MooThing *cur;
	MooAction *action;

	if (!args->thing)
		args->thing = this;
	args->action = name;
	args->result = NULL;
	for (cur = this; cur; cur = moo_thing_lookup_id(cur->parent)) {
		if ((action = sdm_tree_find(cur->actions, name)))
			return(action->do_action(this, args));
	}
	return(1);
}

int MooThing::do_abbreved_action(const char *name, MooActionArgs *args)
{
	MooThing *cur;
	MooAction *action;

	if (!args->thing)
		args->thing = this;
	args->action = name;
	args->result = NULL;
	for (cur = this; cur; cur = moo_thing_lookup_id(cur->parent)) {
		if ((action = sdm_tree_find_partial(cur->actions, name)))
			return(action->do_action(this, args));
	}
	return(1);
}


int MooThing::add(MooThing *obj)
{
	if (obj->location == this)
		return(0);
	/** If this object is in another object and it can't be removed, then we don't add it */
	if (obj->location && obj->location->remove(obj))
		return(-1);
	obj->location = this;
	obj->next = NULL;
	if (this->end_objects) {
		this->end_objects->next = obj;
		this->end_objects = obj;
	}
	else {
		this->objects = obj;
		this->end_objects = obj;
	}
	return(0);
}

int MooThing::remove(MooThing *obj)
{
	MooThing *cur, *prev;

	for (prev = NULL, cur = this->objects; cur; prev = cur, cur = cur->next) {
		if (cur == obj) {
			if (prev)
				prev->next = cur->next;
			else
				this->objects = cur->next;
			if (this->end_objects == cur)
				this->end_objects = prev;
			cur->location = NULL;
			return(0);
		}
	}
	return(1);
}



int moo_thing_assign_id(MooThing *thing, moo_id_t id)
{
	MooThing **tmp;

	/** If the thing already has a valid id, remove it's existing entry in the table.  If an error
	    happens in this function, the thing's id will be -1 in case the caller doesn't check */
	if ((thing->id > 0) && (thing->id < moo_thing_table_size)) {
		moo_thing_table[thing->id] = NULL;
		thing->id = -1;
	}

	/** If the new id is invalid, then don't do anything.  This maximum size is to prevent us from
	    inadvertanly creating a gigantic table */
	if ((id < 0) || (id >= THING_MAX_TABLE_SIZE))
		return(-1);

	// TODO convert this to use new/delete?
	/** Increase the size of the table if id is too big to have a spot in the table */
	if (id >= moo_thing_table_size) {
		if (!(tmp = (MooThing **) memory_realloc(moo_thing_table, sizeof(struct sdm_thing *) * (id + THING_TABLE_EXTRA_SIZE))))
			return(-1);
		moo_thing_table = tmp;
		memset(moo_thing_table + moo_thing_table_size, '\0', sizeof(struct sdm_thing *) * (id + THING_TABLE_EXTRA_SIZE - moo_thing_table_size));
		moo_thing_table_size = id + THING_TABLE_EXTRA_SIZE;
	}

	// TODO should we destroy the object or prevent the new object from taking that ID?
	//if (moo_thing_table[id])
	//	destroy_sdm_object(SDM_OBJECT(moo_thing_table[id]));
	/** If there is already a thing with the same id, then don't make the assignmet */
	if (moo_thing_table[id])
		return(-1);
	thing->id = id;
	moo_thing_table[id] = thing;

	// TODO for now we always make the next id the largest unassign id
	if (next_id <= id)
		next_id = id + 1;
	return(0);
}

int moo_thing_assign_new_id(struct sdm_thing *thing)
{
	// TODO for now we will not assign the ids of holes in the table
	/** Keep in mind that as it stands, the id of the parent of an object must be smaller than the id
	    of that object itself or the loading of the object will fail. */
	return(moo_thing_assign_id(thing, next_id++));
}


