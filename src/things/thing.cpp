/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/tasks/task.h>
#include <sdm/things/user.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/args.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/funcs/func.h>
#include <sdm/funcs/method.h>
#include <sdm/things/thing.h>


#define THING_TABLE_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE
#define THING_PROPERTIES_SIZE		5
#define THING_METHODS_SIZE		5

#define MOO_THING_INIT_SIZE			100
/// This is to prevent us from making giant table accidentally
#define MOO_THING_MAX_SIZE			65536

MooObjectType moo_thing_obj_type = {
	NULL,
	"thing",
	typeid(MooThing).name(),
	(moo_type_make_t) make_moo_thing
};

MooArray<MooThing *> *moo_thing_table = NULL;

int init_thing(void)
{
	if (moo_thing_table)
		return(1);
	moo_object_register_type(&moo_thing_obj_type);
	moo_thing_table = new MooArray<MooThing *>(MOO_THING_INIT_SIZE, MOO_THING_MAX_SIZE, THING_TABLE_BITS);
	return(0);
}

void release_thing(void)
{
	if (moo_thing_table)
		delete moo_thing_table;
	moo_object_deregister_type(&moo_thing_obj_type);
}

MooObject *make_moo_thing(MooDataFile *data)
{
	int id;
	MooThing *thing;

	id = data->read_integer();
	// TODO is this right to return the nil value, or should you create a new nil value
	if (!(thing = MooThing::lookup(id)))
		return(make_moo_nil(NULL));
	return(thing);
}

MooThing::MooThing(moo_id_t id, moo_id_t parent)
{
	// TODO temporary since we haven't properly dealt with refcounting/thingrefs
	this->set_nofree();

	m_properties = new MooObjectHash();
	// TODO we could choose to only create an actions list when we want to place a new
	//	action in it unique to this object and otherwise, an action on this object will
	//	only send the request to it's parent object
	m_methods = new MooObjectHash();

	/// Set the thing id and add the thing to the table.  If id = MOO_NO_ID, don't add it to a table.
	/// If the id = MOO_NEW_ID then assign the next available id
	m_bits = 0;
	m_id = id;
	if (m_id >= 0) {
		if (!moo_thing_table->set(m_id, this))
			m_id = -1;
	}
	else if (m_id == MOO_NEW_ID)
		m_id = moo_thing_table->add(this);
	m_parent = parent;
}

MooThing::~MooThing()
{
	MOO_DECREF(m_properties);
	MOO_DECREF(m_methods);
	if (m_id >= 0)
		moo_thing_table->set(m_id, NULL);
}

MooThing *MooThing::lookup(moo_id_t id)
{
	MooThing *thing;

	if (id < 0)
		return(NULL);
	if (!(thing = moo_thing_table->get(id))) {
		thing = new MooThing(id);
		thing->load();
	}
	return(thing);
}

int MooThing::read_entry(const char *type, MooDataFile *data)
{
	int res;

	if (!strcmp(type, "properties")) {
		if (data->read_children()) {
			res = m_properties->read_data(data);
			data->read_parent();
		}
	}
	else if (!strcmp(type, "methods")) {
		if (data->read_children()) {
			res = m_methods->read_data(data);
			data->read_parent();
		}
	}
	else if (!strcmp(type, "bits")) {
		int bits = data->read_integer_entry();
		m_bits = bits;
	}
	else if (!strcmp(type, "id")) {
		moo_id_t id = data->read_integer_entry();
		this->assign_id(id);
	}
	else if (!strcmp(type, "parent")) {
		moo_id_t id = data->read_integer_entry();
		m_parent = id;
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooThing::write_data(MooDataFile *data)
{
	data->write_integer(m_id);
	return(0);
}

int MooThing::load()
{
	char buffer[STRING_SIZE];

	if (m_id < 0) {
		moo_status("THING: attempting to read thing with an invalid ID");
		return(-1);
	}
	snprintf(buffer, STRING_SIZE, "objs/%04d.xml", m_id);
	m_properties->clear();
	m_methods->clear();
	return(this->read_file(buffer, "thing"));
}

int MooThing::save()
{
	MooDataFile *data;
	char file[STRING_SIZE];

	if (m_id < 0) {
		moo_status("THING: attempting to write thing with an invalid ID");
		return(-1);
	}
	//snprintf(file, STRING_SIZE, "objs/%04d/%04d.xml", m_id / 10000, m_id % 10000);
	snprintf(file, STRING_SIZE, "objs-temp/%04d.xml", m_id);
	moo_status("Writing thing data to file \"%s\".", file);
	data = new MooDataFile(file, MOO_DATA_WRITE, "thing");

	data->write_integer_entry("id", m_id);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_bits != 0)
		data->write_hex_entry("bits", m_bits);
	MooObject::write_data(data);

	/// Write the properties to the file
	if (m_properties->entries()) {
		data->write_begin_entry("properties");
		m_properties->write_data(data);
		data->write_end_entry();
	}

	/// Write the actions to the file
	if (m_methods->entries()) {
		data->write_begin_entry("methods");
		m_methods->write_data(data);
		data->write_end_entry();
	}

	delete data;
	return(0);
}

int MooThing::save_all()
{
	MooThing *thing;

	for (int i = 0; i <= moo_thing_table->last(); i++) {
		thing = moo_thing_table->get(i);
		if (!thing || dynamic_cast<MooUser *>(thing))
			continue;
		thing->save();
	}
	return(0);
}

int MooThing::to_string(char *buffer, int max)
{
	return(snprintf(buffer, max, "#%d", m_id));
}

MooObject *MooThing::access_property(const char *name, MooObject *value)
{
	// TODO do you need to do permissions checks here?
	if (value) {
		if (this->set_property(name, value) < 0)
			return(NULL);
		return(value);
	}
	else
		return(this->get_property(name, NULL));
}

MooObject *MooThing::access_method(const char *name, MooObject *value)
{
	// TODO do you need to do permissions checks here?
	if (value) {
		if (this->set_method(name, value) < 0)
			return(NULL);
		return(value);
	}
	else
		return(this->get_method(name));
}

///// Property Methods /////

int MooThing::set_property(const char *name, MooObject *obj)
{
	MooObject *cur;

	// TODO should there be a way to store the property in the local object
	if (!name || (*name == '\0'))
		return(-1);
	/// If the object is NULL, remove the entry from the table
	if ((cur = m_properties->get(name))) {
		if (!obj) {
			// TODO no perms checks for now
			//this->check_throw(MOO_PERM_W);
			m_properties->remove(name);
			return(1);
		}
		else {
			// TODO no perms checks for now
			//cur->check_throw(MOO_PERM_W);
			obj->owner(cur->owner());
			obj->permissions(cur->permissions());
			// TODO you could also do a check here for the type (only allow the same type to overwrite)
			return(m_properties->set(name, MOO_INCREF(obj)));
		}
	}
	else {
		if (!obj)
			return(1);
		// TODO no perms checks for now
		//this->check_throw(MOO_PERM_W);
		return(m_properties->set(name, MOO_INCREF(obj)));
	}
}

MooObject *MooThing::get_property(const char *name, MooObjectType *type)
{
	MooObject *obj;

	if (!(obj = this->get_property_raw(name, NULL)))
		return(NULL);
	// TODO no perms checks for now
	//obj->check_throw(MOO_PERM_R);
	if (!type || obj->is_a(type))
		return(obj);
	return(NULL);
}

MooObject *MooThing::get_property_raw(const char *name, MooThing **thing)
{
	MooThing *cur;
	MooObject *obj;

	for (cur = this; cur; cur = cur->parent()) {
		if ((obj = cur->m_properties->get(name))) {
			if (thing)
				*thing = cur;
			return(obj);
		}
	}
	return(NULL);
}


///// Method Methods /////

int MooThing::set_method(const char *name, MooObject *action)
{
	if (!name || (*name == '\0'))
		return(-1);
	// TODO do permissions check??
	/// If the action is NULL, remove the entry from the table
	if (!action)
		return(m_methods->remove(name));
	return(m_methods->set(name, MOO_INCREF(action)));
}

MooObject *MooThing::get_method(const char *name)
{
	MooThing *cur;
	MooObject *func;

	// TODO do permissions check??
	for (cur = this; cur; cur = cur->parent()) {
		if ((func = cur->m_methods->get(name)))
			return(func);
	}
	return(NULL);
}

///// Helper Methods /////

MooThing *MooThing::reference(const char *name)
{
	moo_id_t id;

	if (name[0] == '#') {
		id = ::atoi(&name[1]);
		if (id <= 0 || id > MOO_THING_MAX_SIZE)
			return(NULL);
		return(MooThing::lookup(id));
	}
	// TODO should we just get rid of this entirely?
	else if (name[0] == '$') {
		// TODO should we break up the reference??
		MooObject *ref = global_env->get(&name[1]);
		if (!ref)
			return(NULL);
		return(ref->get_thing());
	}
	return(NULL);
}

int MooThing::is_wizard(moo_id_t id)
{
	MooThing *thing;

	if (!(thing = MooThing::lookup(id)))
		return(0);
	return(thing->is_wizard());
}

int MooThing::assign_id(moo_id_t id)
{
	/// If the thing already has an ID, then remove it from the table
	if (this->m_id >= 0)
		moo_thing_table->set(m_id, NULL);
	/// Assign the thing to the appropriate index in the table and set the ID if it succeeded
	m_id = -1;
	if (id >= 0) {
		if (moo_thing_table->set(id, this))
			m_id = id;
	}
	else
		// TODO should this only assign if the ID is -2 (MOO_NEW_ID)
		m_id = moo_thing_table->add(this);
	if (m_id < 0)
		moo_status("Error: Attempted to reassign ID, %d", id);
	return(m_id);
}

const char *MooThing::name()
{
	MooObject *obj;
	const char *name = NULL;

	try {
		if (!(obj = this->get_property("name", NULL)))
			return(NULL);
		name = obj->get_string();
	}
	catch (...) { }

	if (!name)
		return("???");
	return(name);
}

MooThing *MooThing::location()
{
	MooObject *obj;
	if (!(obj = this->resolve_property("location")))
		return(NULL);
	return(dynamic_cast<MooThing *>(obj));
}

int MooThing::move(MooThing *where)
{
	MooThing *was;
	MooObjectArray *contents;

	// TODO add permissions checks (this wasn't working)
	//this->check_throw(MOO_PERM_W);
	if ((was = this->location()) && (contents = was->contents()))
		contents->remove(this);
	if ((contents = where->contents())) {
		// TODO do you need to check permissions on the contents array to be sure you can write to them?
		if (contents->add(this) < 0)
			return(-1);
		this->resolve_property("location", where);
	}
	return(0);
}

MooThing *MooThing::get_channel(const char *name)
{
	MooThing *channels;
	MooObjectHash *list;

	if (!(channels = MooThing::reference(MOO_CHANNELS))) {
		moo_status("CHANNEL: channels object not found");
		return(NULL);
	}
	if (!(list = dynamic_cast<MooObjectHash *>(channels->resolve_property("list")))) {
		moo_status("CHANNEL: channel list not found on channels object");
		return(NULL);
	}
	return(dynamic_cast<MooThing *>(list->get(name)));
}

/*
int MooChannel::valid_channelname(const char *name)
{
	int i;

	if (name[0] != '#')
		return(0);
	for (i = 1; name[i] != '\0'; i++) {
		if (!(((name[i] >= 0x30) && (name[i] <= 0x39))
		    || ((name[i] >= 0x41) && (name[i] <= 0x5a))
		    || ((name[i] >= 0x61) && (name[i] <= 0x7a))
		    || (name[i] == '-') || (name[i] == '.') || (name[i] == '_')))
			return(0);
	}
	return(1);
}
*/


