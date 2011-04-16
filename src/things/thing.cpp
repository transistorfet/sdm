/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/tasks/task.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

#include <sdm/objs/object.h>
#include <sdm/objs/args.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>

#include <sdm/actions/method.h>

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
	(moo_type_create_t) moo_thing_create
};

MooObjectHash *moo_global_table = NULL;
MooArray<MooThing *> *moo_thing_table = NULL;

int init_thing(void)
{
	if (moo_thing_table)
		return(1);
	moo_object_register_type(&moo_thing_obj_type);
	// TODO should the global table be read/written to a file?
	moo_global_table = new MooObjectHash();
	moo_thing_table = new MooArray<MooThing *>(MOO_THING_INIT_SIZE, MOO_THING_MAX_SIZE, THING_TABLE_BITS);
	return(0);
}

void release_thing(void)
{
	if (moo_global_table)
		delete moo_global_table;
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
	MOO_DECREF(m_properties);
	MOO_DECREF(m_methods);
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
	const MooObjectType *objtype;

	if (!strcmp(type, "properties")) {
		data->read_children();
		res = m_properties->read_data(data);
		data->read_parent();
	}
	else if (!strcmp(type, "methods")) {
		data->read_children();
		res = m_methods->read_data(data);
		data->read_parent();
		// TODO should you go through all the methods and ->init() them, so that the object is set properly?
	}
	else if (!strcmp(type, "thing")) {
		MooThing *thing = NULL;
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type((*buffer != '\0') ? buffer : "thing", &moo_thing_obj_type))) {
			moo_status("THING: Unable to find thing type, %s", buffer);
			return(-1);
		}
		if (!(thing = (MooThing *) moo_make_object(objtype))) {
			moo_status("THING: Error loading thing.");
			return(-1);
		}
		data->read_children();
		thing->read_data(data);
		data->read_parent();
		this->add(thing);
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
	else if (!strcmp(type, "location")) {
		moo_id_t id = data->read_integer_entry();
		MooThing *thing = MooThing::lookup(id);
		if (thing && this != thing)
			thing->add(this);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooThing::write_data(MooDataFile *data)
{
	data->write_integer_entry("id", m_id);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_bits != 0)
		data->write_hex_entry("bits", m_bits);
	MooObject::write_data(data);
	if (m_location)
		data->write_integer_entry("location", m_location->m_id);

	/// Write the properties to the file
	data->write_begin_entry("properties");
	m_properties->write_data(data);
	data->write_end_entry();

	/// Write the actions to the file
	data->write_begin_entry("methods");
	m_methods->write_data(data);
	data->write_end_entry();

	/// Write the things we contain to the file
	for (MooThing *cur = m_objects; cur; cur = cur->m_next) {
		if (cur->is_a(&moo_user_obj_type))
			continue;
		else if (cur->is_a(&moo_world_obj_type)) {
			cur->write_data(data);
		}
		else {
			data->write_begin_entry("thing");
			if (cur->type() != &moo_thing_obj_type)
				data->write_attrib_string("type", cur->type_name());
			cur->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

/*
// TODO you should probably move these to basics.cpp or builder.cpp, or make them accept user/channel args
MooThing *MooThing::create(moo_id_t parent)
{
	MooThing *thing;

	// TODO how do we know if this fails?  we should destroy the object if it does
	// TODO check if current task owner has create permissions
	thing = new MooThing(MOO_NEW_ID, parent);
	thing->init();
	thing->moveto(thing->owner_thing());
	return(thing);
}

MooThing *MooThing::clone()
{
	MooThing *thing;

	// TODO check if current task owner has create permissions
	// TODO check if the parent object is cloneable
	thing = new MooThing(MOO_NEW_ID, m_parent);
	// TODO how do we know if this fails?  we should destroy the object if it does

	// TODO copy all properties
	// TODO copy all actions

	//thing->permissions(THING_DEFAULT_PERMISSIONS);
	// TODO do we call init at any point??

	thing->moveto(thing->owner_thing());
	return(thing);
}
*/

void MooThing::add_global(const char *name, MooObject *obj)
{
	moo_global_table->set(name, obj);
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
	MooThing *thing;

	// TODO should there be a way to store the property in the local object
	if (!name || (*name == '\0'))
		return(-1);
	/// If the object is NULL, remove the entry from the table
	if ((cur = this->get_property_raw(name, &thing))) {
		if (!obj) {
			thing->check_throw(MOO_PERM_W);
			thing->m_properties->remove(name);
			return(1);
		}
		else {
			cur->check_throw(MOO_PERM_W);
			obj->owner(cur->owner());
			obj->permissions(cur->permissions());
			// TODO you could also do a check here for the type (only allow the same type to overwrite)
			return(thing->m_properties->set(name, MOO_INCREF(obj)));
		}
	}
	else {
		if (!obj)
			return(1);
		this->check_throw(MOO_PERM_W);
		return(m_properties->set(name, MOO_INCREF(obj)));
	}
}

MooObject *MooThing::get_property(const char *name, MooObjectType *type)
{
	MooObject *obj;

	if (!(obj = this->get_property_raw(name, NULL)))
		return(NULL);
	obj->check_throw(MOO_PERM_R);
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

int MooThing::convert_result(MooObject *&result, int def)
{
	int res;

	if (result) {
		res = result->get_integer();
		MOO_DECREF(result);
		result = NULL;
	}
	else
		res = def;
	return(res);
}


///// Search Methods /////

MooThing *MooThing::find(const char *name)
{
	MooThing *thing;

	if ((thing = MooThing::reference(name)))
		return(thing);
	else if (!strcmp(name, "me"))
		return(this);
	else if (!strcmp(name, "here") && m_location)
		return(m_location);
	else if ((thing = this->find_named(name)))
		return(thing);
	else if (m_location && (thing = m_location->find_named(name)))
		return(thing);
	return(NULL);
}

MooThing *MooThing::find_named(const char *name)
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
		if (len <= 0)
			return(NULL);
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
		id = ::atoi(&name[1]);
		if (id <= 0 || id > MOO_THING_MAX_SIZE)
			return(NULL);
		return(moo_thing_table->get(id));
	}
	else if (name[0] == '/') {
		MooWorld *world = MooWorld::root();
		if (!world)
			return(NULL);
		return(world->find_named(name));
	}
	// TODO should we just get rid of this entirely?
	else if (name[0] == '$') {
		// TODO should we break up the reference??
		MooObject *ref = moo_global_table->get(&name[1]);
		if (!ref)
			return(NULL);
		return(ref->get_thing());
	}
	return(NULL);
}

///// Helper Methods /////

// TODO all these helper functions should be put into MooCode or else made into hardcoded functions called via a do_evaluate()

int MooThing::notify(int type, MooThing *thing, MooThing *channel, const char *text)
{
	MooArgs args(2, thing, channel);

	args.m_args->set(0, new MooNumber((long int) type));
	args.m_args->set(1, new MooString(text));
	return(this->call_method(channel, this->resolve_method("notify"), &args));
}

int MooThing::notifyf(int type, MooThing *thing, MooThing *channel, const char *fmt, ...)
{
	va_list va;
	char buffer[LARGE_STRING_SIZE];

	va_start(va, fmt);
	vsnprintf(buffer, LARGE_STRING_SIZE, fmt, va);
	return(this->notify(type, thing, channel, buffer));
}

int MooThing::notify(int type, MooArgs *args, const char *fmt, ...)
{
	va_list va;
	char buffer2[LARGE_STRING_SIZE];

	{
		// TODO should you have this function with va? or should you have it without, or both?  Same for MooThing::expand_str
		MooObjectHash *env;
		char buffer1[LARGE_STRING_SIZE];

		if (args)
			env = args->make_env();
		va_start(va, fmt);
		vsnprintf(buffer1, LARGE_STRING_SIZE, fmt, va);
		MooObject::format(buffer2, LARGE_STRING_SIZE, env, buffer1);
		MOO_DECREF(env);
	}
	return(this->notify(type, args->m_user, args->m_channel, buffer2));
}

int MooThing::notify_all(int type, MooThing *thing, MooThing *channel, const char *text)
{
	MooThing *cur;

	// TODO with the new channel system, this would be different depending on the type of channel...  Should both types
	//	of channels be defined in C++? or a common channel C++ object with the specifics and actions different?

	for (cur = this->contents(); cur; cur = cur->next()) {
		cur->notify(type, thing, channel, text);
	}
	return(0);
}

int MooThing::notify_all_except(MooThing *except, int type, MooThing *thing, MooThing *channel, const char *text)
{
	MooThing *cur;

	for (cur = this->contents(); cur; cur = cur->next()) {
		if (cur != except)
			cur->notify(type, thing, channel, text);
	}
	return(0);
}

int MooThing::moveto(MooThing *user, MooThing *channel, MooThing *to)
{
	if (!to)
		return(-1);
	if (to == m_location)
		return(0);
	try {
		{
			MooArgs args(1, user, channel);
			args.m_args->set(0, new MooThingRef(m_id));
			if (to->call_method(channel, to->resolve_method("accept"), &args) < 0)
				return(-1);
			if (MooThing::convert_result(args.m_result) != 1)
				return(1);
		}

		if (m_location) {
			MooArgs args(1, user, channel);
			args.m_args->set(0, new MooThingRef(m_id));
			if (m_location->call_method(channel, m_location->resolve_method("do_exit"), &args) < 0)
				return(-1);
			if (MooThing::convert_result(args.m_result) < 0)
				return(1);
		}

		if (to->add(this) < 0)
			return(1);

		if (m_location) {
			MooArgs args(1, user, channel);
			args.m_args->set(0, new MooThingRef(m_id));
			return(m_location->call_method(channel, m_location->resolve_method("do_enter"), &args));
		}
		return(0);
	}
	catch (...) {
		// TODO print error
		return(-1);
	}
}

int MooThing::attach_orphans()
{
	MooThing *cur;
	MooThing *root;

	// TODO should this instead be a sign that we need to recycle these things?
	if (!(root = MooWorld::root()))
		return(-1);
	for (int i = 0; i <= moo_thing_table->last(); i++) {
		if ((cur = moo_thing_table->get(i))) {
			if (!cur->m_location)
				root->add(cur);
		}
	}
	return(0);
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

int MooThing::add(MooThing *obj)
{
	if (obj->m_location == this)
		return(0);
	/// Make sure this addition doesn't create a loop
	for (MooThing *cur = this; cur; cur = cur->m_location) {
		if (cur == obj) {
			moo_status("THING: Adding thing will create loop, %d", obj->id());
			return(-1);
		}
	}

	/// If this object is in another object and it can't be removed, then we don't add it
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

///// Accessors /////

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


