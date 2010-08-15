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
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/thing.h>

#define THING_PROPERTIES_BITS		MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL | MOO_HBF_DELETE
#define THING_PROPERTIES_SIZE		5
#define THING_ACTIONS_BITS		MOO_TBF_REPLACE | MOO_TBF_REMOVE | MOO_TBF_DELETEALL | MOO_TBF_DELETE

#define MOO_THING_INIT_SIZE			100
/** This is to prevent us from making giant table accidentally */
#define MOO_THING_MAX_SIZE			65536

MooObjectType moo_thing_obj_type = {
	NULL,
	"thing",
	typeid(MooThing).name(),
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

MooThing::MooThing(moo_id_t id, moo_id_t parent)
{
	m_properties = new MooHash<MooObject *>(THING_PROPERTIES_SIZE, THING_PROPERTIES_BITS);
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	m_actions = new MooTree<MooAction *>(THING_ACTIONS_BITS);

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
	m_location = NULL;
	m_next = NULL;
	m_objects = NULL;
	m_end_objects = NULL;
}

MooThing::~MooThing()
{
	MooThing *cur, *tmp;

	if (m_location)
		m_location->remove(this);
	if (m_properties)
		delete m_properties;
	if (m_actions)
		delete m_actions;
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
	const MooObjectType *objtype;

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
			moo_status("Error loading action, %s.", buffer);
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
	MooHashEntry<MooObject *> *hentry;
	MooTreeEntry<MooAction *> *tentry;

	data->write_integer_entry("id", m_id);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_location)
		data->write_integer_entry("location", m_location->m_id);

	/** Write the properties to the file */
	m_properties->reset();
	while ((hentry = m_properties->next_entry())) {
		data->write_begin_entry("property");
		data->write_attrib("type", hentry->m_data->type_name());
		data->write_attrib("name", hentry->m_key);
		hentry->m_data->write_data(data);
		data->write_end_entry();
	}

	/** Write the actions to the file */
	m_actions->reset();
	while ((tentry = m_actions->next_entry())) {
		data->write_begin_entry("action");
		data->write_attrib("type", tentry->m_data->type_name());
		data->write_attrib("name", tentry->m_key);
		tentry->m_data->write_data(data);
		data->write_end_entry();
	}

	/** Write the things we contain to the file */
	for (MooThing *cur = m_objects; cur; cur = cur->m_next) {
		if (cur->is_a(&moo_user_obj_type))
			continue;
		else if (cur->is_a(&moo_world_obj_type)) {
			cur->write_data(data);
		}
		else {
			data->write_begin_entry("thing");
			cur->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

MooThing *MooThing::create(MooUser *user, moo_id_t parent)
{
	MooThing *thing;

	// TODO create a new object and fully initialze it
	thing = new MooThing(-1, parent);
	// TODO how do we know if this fails?  we should destroy the object if it does
	thing->set_property("owner", new MooNumber(user->m_id));
	//thing->permissions(THING_DEFAULT_PERMISSIONS);

	//thing->moveto(user, user);

	return(thing);
}

int MooThing::set_property(const char *name, MooObject *obj)
{
	if (!name || (*name == '\0'))
		return(-1);
	/** If the object is NULL, remove the entry from the table */
	if (!obj) {
		m_properties->remove(name);
		return(1);
	}
	return(m_properties->set(name, obj));
}

int MooThing::set_property(const char *name, moo_id_t id)
{
	MooThingRef *obj;

	obj = new MooThingRef(id);
	return(this->set_property(name, obj));
}

int MooThing::set_property(const char *name, double num)
{
	MooNumber *obj;

	obj = new MooNumber(num);
	return(this->set_property(name, obj));
}

int MooThing::set_property(const char *name, const char *str)
{
	MooString *obj;

	obj = new MooString(str);
	return(this->set_property(name, obj));
}

MooObject *MooThing::get_property(const char *name, MooObjectType *type)
{
	MooThing *cur;
	MooObject *obj;

	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if (!(obj = m_properties->get(name)))
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
		return(m_actions->remove(name));
	return(m_actions->set(name, action));
}

int MooThing::do_action(MooAction *action, MooArgs *args)
{
	try {
		// TODO add permissions checks
		// TODO is this right, deleting the result if it's present?
		if (args->m_result)
			delete args->m_result;
		args->m_result = NULL;
		args->m_this = this;
		return(action->do_action(this, args));
	}
	catch (...) {
		moo_status("An unspecified error has occured during \"%s\", name");
		return(-1);
	}
}

int MooThing::do_action(const char *name, MooArgs *args)
{
	MooThing *cur;
	MooAction *action;

	// TODO should this be set here or should it just be in MooAction or something
	args->m_action = name;
	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if ((action = cur->m_actions->get(name)))
			return(cur->do_action(action, args));
	}
	return(MOO_ACTION_NOT_FOUND);
}

int MooThing::do_abbreved_action(const char *name, MooArgs *args)
{
	MooThing *cur;
	MooAction *action;

	args->m_action = name;
	for (cur = this; cur; cur = MooThing::lookup(cur->m_parent)) {
		if ((action = cur->m_actions->get_partial(name)))
			return(cur->do_action(action, args));
	}
	return(MOO_ACTION_NOT_FOUND);
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

int MooThing::moveto(MooThing *thing, MooThing *by)
{
	// TODO fill this in
	// TODO this will check permissions of via to perform the action (??) and then
	//	call various actions on the objects to actually do the move
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
	return(m_id);
}

MooThing *MooThing::find(const char *name)
{
	int len;
	MooString *str;
	const char *end;
	MooThing *cur, *thing;

	if (name[0] == '/') {
		thing = (MooThing *) MooWorld::root();
		name = &name[1];
	}
	else
		thing = this;

	while (thing) {
		if ((end = strchr(name, '/')))
			len = end - name;
		else
			len = strlen(name);
		for (cur = thing->m_objects; cur; cur = cur->m_next) {
			// TODO modify this to use the aliases list or something
			if (!(str = (MooString *) cur->get_property("name", &moo_string_obj_type)))
				continue;
			if (!str->compare(name, len)) {
				if (name[len] == '\0')
					return(cur);
				else {
					thing = cur;
					name = &name[len + 1];
					break;
				}
			}
		}
		if (!cur)
			return(NULL);
	}
	return(NULL);
}

MooThing *MooThing::reference(const char *name)
{
	moo_id_t id;

	if (name[0] == '#') {
		id = atoi(&name[1]);
		if (id <= 0 || id > MOO_THING_MAX_SIZE)
			return(NULL);
		return(moo_thing_table->get(id));
	}
	else if (name[0] == '/') {
		MooWorld *world = MooWorld::root();
		if (!world)
			return(NULL);
		return(world->find(name));
	}
	else if (name[0] == '$')
		;// TODO do some kind of lookup by variable name (where is this table?)
	return(NULL);
}

