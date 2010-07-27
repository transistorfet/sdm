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

#define MOO_THING_INIT_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define MOO_THING_MAX_SIZE			65536

MooObjectType moo_thing_obj_type = {
	NULL,
	"thing",
	(moo_type_create_t) moo_thing_create
};

MooArray<MooThing *> *moo_thing_table = NULL;

int init_thing(void)
{
	if (moo_thing_table)
		return(1);
	if (moo_object_register_type(&moo_thing_obj_type) < 0)
		return(-1);
	moo_thing_table = new MooArray<MooThing *>(MOO_THING_INIT_SIZE, MOO_THING_MAX_SIZE);
	return(0);
}

void release_thing(void)
{
	if (moo_thing_table)
		delete moo_thing_table;
	moo_object_deregister_type(&moo_thing_obj_type);
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
	if (!(m_properties = create_sdm_hash(0, THING_INIT_PROPERTIES, (destroy_t) destroy_sdm_object)))
		throw -1;
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	if (!(m_actions = create_sdm_tree(0, (destroy_t) destroy_sdm_object)))
		throw -1;

	/** Set the thing id and add the thing to the table.  If id = SDM_NO_ID, don't add it to a table.
	    If the id = SDM_NEW_ID then assign the next available id */
	m_id = id;
	if (m_id >= 0) {
		if (!moo_thing_table->set(m_id, this))
			m_id = -1;
	}
	else if (m_id == SDM_NEW_ID)
		moo_thing_table->add(this);
	m_parent = parent;
}

MooThing::~MooThing()
{
	MooThing *cur, *tmp;

	if (m_location)
		m_location->remove(this);
	if (m_properties)
		destroy_sdm_hash(m_properties);
	if (m_actions)
		destroy_sdm_tree(m_actions);
	if (m_id >= 0)
		moo_thing_table->set(m_id, NULL);

	for (cur = m_objects; cur; cur = tmp) {
		tmp = cur->m_next;
		delete cur;
	}
}

int MooThing::read_entry(const char *type, MooDataFile *data)
{
	int res;
	char buffer[STRING_SIZE];
	MooObject *obj = NULL;
	MooObjectType *objtype;

	if (!strcmp(type, "property")) {
		data->read_attrib("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type((*buffer != '\0') ? buffer : "string", NULL)))
			return(-1);
		data->read_attrib("name", buffer, STRING_SIZE);
		if (!(obj = moo_make_object(objtype)))
			return(-1);
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set_property(buffer, obj) < 0)) {
			delete obj;
			return(-1);
		}
	}
	else if (!strcmp(type, "thing")) {
		MooThing *thing = new MooThing();
		data->read_children();
		thing->read_data(data);
		data->read_parent();
		this->add(thing);
	}
	else if (!strcmp(type, "action")) {
		data->read_attrib("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, &moo_action_obj_type))
		    || !(obj = moo_make_object(objtype)))
			return(-1);
		data->read_attrib("name", buffer, STRING_SIZE);
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set_action(buffer, (MooAction *) obj) < 0)) {
			sdm_status("Error loading action, %s.", buffer);
			delete obj;
			return(-1);
		}
	}
	else if (!strcmp(type, "id")) {
		moo_id_t id = data->read_integer_entry();
		this->assign_id(id);
	}
	else if (!strcmp(type, "location")) {
		moo_id_t id = data->read_integer_entry();
		MooThing *thing = MooThing::lookup(id);
		if (thing)
			thing->add(this);
	}
	else if (!strcmp(type, "parent")) {
		moo_id_t id = data->read_integer_entry();
		m_parent = id;
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}

int MooThing::write_data(MooDataFile *data)
{
	struct sdm_hash_entry *hentry;
	struct sdm_tree_entry *tentry;

	data->write_integer_entry("id", m_id);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_location)
		data->write_integer_entry("location", m_location->m_id);

	/** Write the properties to the file */
	sdm_hash_traverse_reset(m_properties);
	while ((hentry = sdm_hash_traverse_next_entry(m_properties))) {
		data->write_begin_entry("property");
		data->write_attrib("type", ((MooObject *) hentry->data)->m_type->m_name);
		data->write_attrib("name", hentry->name);
		((MooObject *) hentry->data)->write_data(data);
		data->write_end_entry();
	}

	/** Write the actions to the file */
	sdm_tree_traverse_reset(m_actions);
	while ((tentry = sdm_tree_traverse_next_entry(m_actions))) {
		data->write_begin_entry("action");
		data->write_attrib("type", ((MooObject *) tentry->data)->m_type->m_name);
		data->write_attrib("name", tentry->name);
		((MooObject *) tentry->data)->write_data(data);
		data->write_end_entry();
	}

	/** Write the things we contain to the file */
	for (MooThing *cur = m_objects; cur; cur = cur->m_next) {
		if (cur->is_a(&moo_user_obj_type))
			continue;
		else if (cur->is_a(&moo_world_obj_type)) {
			cur->write(data);
			/** If we don't continue here, the world will be written to this file which we don't want
			    because the world object is a reference only and is written to it's own file */
			continue;
		}
		else
			data->write_begin_entry("thing");
		cur->write_data(data);
		data->write_end_entry();
	}
	return(0);
}


int MooThing::set_property(const char *name, MooObject *obj)
{
	if (!name || (*name == '\0'))
		return(-1);
	/** If the object is NULL, remove the entry from the table */
	if (!obj)
		return(sdm_hash_remove(m_properties, name));
	if (sdm_hash_find(m_properties, name))
		return(sdm_hash_replace(m_properties, name, obj));
	return(sdm_hash_add(m_properties, name, obj));
}

MooObject *MooThing::get_property(const char *name, MooObjectType *type)
{
	MooThing *cur;
	MooObject *obj;

	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if (!(obj = sdm_hash_find(m_properties, name)))
			continue;
		if (!type || obj->is_a(type))
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
		return(sdm_tree_remove(m_actions, name));
	if (sdm_tree_find(m_actions, name))
		return(sdm_tree_replace(m_actions, name, action));
	return(sdm_tree_add(m_actions, name, action));
}

int MooThing::do_action(const char *name, MooArgs *args)
{
	MooThing *cur;
	MooAction *action;

	if (!args->thing)
		args->thing = this;
	args->action = name;
	args->result = NULL;
	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if ((action = sdm_tree_find(cur->m_actions, name)))
			return(action->do_action(this, args));
	}
	return(1);
}

int MooThing::do_abbreved_action(const char *name, MooArgs *args)
{
	MooThing *cur;
	MooAction *action;

	if (!args->thing)
		args->thing = this;
	args->action = name;
	args->result = NULL;
	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if ((action = sdm_tree_find_partial(cur->m_actions, name)))
			return(action->do_action(this, args));
	}
	return(1);
}


int MooThing::add(MooThing *obj)
{
	if (obj->m_location == this)
		return(0);
	/** If this object is in another object and it can't be removed, then we don't add it */
	if (obj->m_location && obj->m_location->remove(obj))
		return(-1);
	obj->m_location = this;
	obj->m_next = NULL;
	if (m_end_objects) {
		m_end_objects->m_next = obj;
		m_end_objects = obj;
	}
	else {
		m_objects = obj;
		m_end_objects = obj;
	}
	return(0);
}

int MooThing::remove(MooThing *obj)
{
	MooThing *cur, *prev;

	for (prev = NULL, cur = m_objects; cur; prev = cur, cur = cur->m_next) {
		if (cur == obj) {
			if (prev)
				prev->m_next = cur->m_next;
			else
				m_objects = cur->m_next;
			if (m_end_objects == cur)
				m_end_objects = prev;
			cur->m_location = NULL;
			return(0);
		}
	}
	return(1);
}

int MooThing::assign_id(moo_id_t id)
{
	/** If the thing already has an ID, then remove it from the table */
	if (this->m_id >= 0)
		moo_thing_table->set(m_id, NULL);
	/** Assign the thing to the appropriate index in the table and set the ID if it succeeded */
	if (moo_thing_table->set(id, this))
		m_id = id;
	else
		m_id = -1;
}

