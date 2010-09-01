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
#include <sdm/tasks/task.h>
#include <sdm/things/user.h>
#include <sdm/things/world.h>

#include <sdm/objs/object.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/thing.h>

#define THING_TABLE_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE
#define THING_PROPERTIES_BITS		MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL | MOO_HBF_DELETE
#define THING_PROPERTIES_SIZE		5
#define THING_ACTIONS_BITS		MOO_TBF_REPLACE | MOO_TBF_REMOVE | MOO_TBF_DELETEALL | MOO_TBF_DELETE

#define MOO_THING_INIT_SIZE			100
/// This is to prevent us from making giant table accidentally
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
	moo_thing_table = new MooArray<MooThing *>(MOO_THING_INIT_SIZE, MOO_THING_MAX_SIZE, THING_TABLE_BITS);
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
	const MooObjectType *objtype;

	if (!strcmp(type, "property")) {
		// TODO add status messages when loading a property fails.  It should never fail and if it does, it will
		//	otherwise get silently 'delete' when the XML is written back
		MooObject *obj = NULL;
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type((*buffer != '\0') ? buffer : "string", NULL)))
			return(-1);
		data->read_attrib_string("name", buffer, STRING_SIZE);
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
		MooThing *thing = NULL;
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type((*buffer != '\0') ? buffer : "thing", &moo_thing_obj_type)))
			return(-1);
		if (!(thing = (MooThing *) moo_make_object(objtype)))
			return(-1);
		data->read_children();
		thing->read_data(data);
		data->read_parent();
		this->add(thing);
	}
	else if (!strcmp(type, "action")) {
		// TODO add status messages when loading a property fails.  It should never fail and if it does, it will
		//	otherwise get silently 'delete' when the XML is written back
		MooAction *action = NULL;
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, &moo_action_obj_type))
		    || !(action = (MooAction *) moo_make_object(objtype)))
			return(-1);
		data->read_attrib_string("name", buffer, STRING_SIZE);
		data->read_children();
		res = action->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set_action(buffer, action) < 0)) {
			moo_status("Error loading action, %s.", buffer);
			delete action;
			return(-1);
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
	else if (!strcmp(type, "location")) {
		moo_id_t id = data->read_integer_entry();
		MooThing *thing = MooThing::lookup(id);
		if (thing)
			thing->add(this);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooThing::write_data(MooDataFile *data)
{
	MooHashEntry<MooObject *> *hentry;
	MooTreeEntry<MooAction *> *tentry;

	data->write_integer_entry("id", m_id);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_bits != 0)
		data->write_hex_entry("bits", m_bits);
	MooObject::write_data(data);
	if (m_location)
		data->write_integer_entry("location", m_location->m_id);

	/// Write the properties to the file
	m_properties->reset();
	while ((hentry = m_properties->next_entry())) {
		data->write_begin_entry("property");
		data->write_attrib_string("type", hentry->m_data->type_name());
		data->write_attrib_string("name", hentry->m_key);
		hentry->m_data->write_data(data);
		data->write_end_entry();
	}

	/// Write the actions to the file
	m_actions->reset();
	while ((tentry = m_actions->next_entry())) {
		data->write_begin_entry("action");
		data->write_attrib_string("type", tentry->m_data->type_name());
		data->write_attrib_string("name", tentry->m_key);
		tentry->m_data->write_data(data);
		data->write_end_entry();
	}

	/// Write the things we contain to the file
	for (MooThing *cur = m_objects; cur; cur = cur->m_next) {
		if (cur->is_a(&moo_user_obj_type))
			continue;
		else if (cur->is_a(&moo_world_obj_type)) {
			cur->write_data(data);
		}
		else {
			data->write_begin_entry("thing");
			if (tentry->m_data->type() != &moo_thing_obj_type)
				data->write_attrib_string("type", tentry->m_data->type_name());
			cur->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

int MooThing::init()
{
	//this->permissions(THING_DEFAULT_PERMISSIONS);

	/// This is a rare situation where we will use the owner of the object rather than the current owner.  This
	/// *shouldn't* make a difference here, since normally they would be the same.
	// TODO is this call correct?
	this->do_action("init", this->owner_thing());
	return(0);
}

MooThing *MooThing::create(moo_id_t parent)
{
	MooThing *thing;

	// TODO how do we know if this fails?  we should destroy the object if it does
	// TODO check if current task owner has create permissions
	thing = new MooThing(MOO_NEW_ID, parent);
	thing->init();
	thing->moveto(thing->owner_thing(), NULL);
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

	thing->moveto(thing->owner_thing(), NULL);
	return(thing);
}

///// Property Methods /////

int MooThing::set_property(const char *name, MooObject *obj)
{
	// TODO do permissions check??
	if (!name || (*name == '\0'))
		return(-1);
	/// If the object is NULL, remove the entry from the table
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

	// TODO do permissions check??
	for (cur = this; cur; cur = cur->parent()) {
		if (!(obj = cur->m_properties->get(name)))
			continue;
		if (!type || obj->is_a(type))
			return(obj);
	}
	return(NULL);
}

moo_id_t MooThing::get_thing_property(const char *name)
{
	MooThingRef *obj;

	if (!(obj = (MooThingRef *) this->get_property(name, &moo_thingref_obj_type)))
		return(-1);
	return(obj->m_id);
}

double MooThing::get_number_property(const char *name)
{
	MooNumber *obj;

	if (!(obj = (MooNumber *) this->get_property(name, &moo_number_obj_type)))
		return(0);
	return(obj->m_num);
}

const char *MooThing::get_string_property(const char *name)
{
	MooString *obj;

	if (!(obj = (MooString *) this->get_property(name, &moo_string_obj_type)))
		return(NULL);
	return(obj->m_str);
}

///// Action Methods /////

int MooThing::set_action(const char *name, MooAction *action)
{
	if (!name || (*name == '\0'))
		return(-1);
	// TODO do permissions check??
	/// If the action is NULL, remove the entry from the table
	if (!action)
		return(m_actions->remove(name));
	action->init(name, this);
	return(m_actions->set(name, action));
}

MooAction *MooThing::get_action(const char *name)
{
	MooThing *cur;
	MooAction *action;

	// TODO do permissions check??
	for (cur = this; cur; cur = cur->parent()) {
		if ((action = cur->m_actions->get(name)))
			return(action);
	}
	return(NULL);
}

MooAction *MooThing::get_action_partial(const char *name)
{
	MooThing *cur;
	MooAction *action;

	for (cur = this; cur; cur = cur->parent()) {
		if ((action = cur->m_actions->get_partial(name)))
			return(action);
	}
	return(NULL);
}

int MooThing::do_action(MooAction *action, MooArgs *args)
{
	try {
		action->check_throw(MOO_PERM_X);

		// TODO is this right, deleting the result if it's present?
		if (args->m_result)
			delete args->m_result;
		args->m_result = NULL;
		args->m_this = this;
		args->m_action = action;

		// TODO should 'this' here instead be action->m_thing??  should there be something like 'this' at all?
		//	It should now be set in the action so we can get it from there
		if (action->permissions() & MOO_PERM_SUID)
			return(MooTask::elevated_do_action(action, this, args));
		else
			return(action->do_action(this, args));
	}
	catch (MooException e) {
		moo_status("ACTION: (%s) %s", action->name(), e.get());
		// TODO should it print this message to the user, also?
		return(-1);
	}
	catch (...) {
		moo_status("%s: An unspecified error has occured", action->name());
		return(-1);
	}
}

int MooThing::do_action(const char *name, MooArgs *args)
{
	MooAction *action;

	if ((action = this->get_action(name)))
		return(this->do_action(action, args));
	return(MOO_ACTION_NOT_FOUND);
}

int MooThing::do_action(const char *name, MooThing *user, MooThing *object, MooThing *target)
{
	MooArgs args;
	MooAction *action;

	// TODO change this with a MooArgs method for setting
	args.m_user = user;
	args.m_caller = (MooThing *) user;
	args.m_object = object;
	args.m_target = target;
	args.m_text = NULL;

	if ((action = this->get_action(name)))
		return(this->do_action(action, &args));
	return(MOO_ACTION_NOT_FOUND);
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
	else if (name[0] == '$')
		;// TODO do some kind of lookup by variable name (where is this table?)
	return(NULL);
}

///// Helper Methods /////

int MooThing::command(const char *text)
{
	int i;
	MooArgs args;
	char buffer[LARGE_STRING_SIZE];

	strcpy(buffer, text);
	i = args.parse_whitespace(buffer);
	args.m_action_text = &buffer[i];
	i += args.parse_word(buffer);
	args.m_text = &text[i];
	args.parse_args(this, args.m_action_text, &buffer[i]);
	return(this->command(args.m_action_text, &args));
}

int MooThing::command(const char *action, const char *text)
{
	MooArgs args;
	char buffer[LARGE_STRING_SIZE];

	strcpy(buffer, text);
	args.parse_args(this, action, buffer);
	args.m_text = text;
	// TODO should there be a command(&args) rather than taking the action parameter?? (i guess 2 functions)
	//	The issue appears to be that we didn't have action_text, and that action must be looked up on each
	//	object, since we don't have a definite 'this' at this point (could be user, location, object, target)
	return(this->command(action, &args));
}

int MooThing::command(const char *action, MooThing *object, MooThing *target)
{
	MooArgs args;

	// TODO change this with a MooArgs method for setting
	args.m_user = this;
	args.m_caller = (MooThing *) this;
	args.m_object = object;
	args.m_target = target;
	args.m_text = NULL;
	return(this->command(action, &args));
}

int MooThing::command(const char *action, MooArgs *args)
{
	int res;

	if ((res = this->do_action(action, args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (m_location && (res = m_location->do_action(action, args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (args->m_object && (res = args->m_object->do_action(action, args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	if (args->m_target && (res = args->m_target->do_action(action, args)) != MOO_ACTION_NOT_FOUND)
		return(res);
	return(MOO_ACTION_NOT_FOUND);
}

int MooThing::print(MooArgs *args, const char *str)
{
	char buffer[LARGE_STRING_SIZE];

	MooThing::expand_str(buffer, LARGE_STRING_SIZE, args, str);
	return(this->notify(TNT_STATUS, NULL, NULL, buffer));
}

/*
int MooThing::print(MooThing *channel, MooThing *thing, const char *text)
{
	if (!m_task)
		return(-1);
	return(m_task->notify(channel, thing, TNT_SAY, text));
}

int MooThing::printf(MooThing *channel, MooThing *thing, const char *fmt, ...)
{
	va_list va;
	char buffer[LARGE_STRING_SIZE];

	if (!m_task)
		return(-1);
	va_start(va, fmt);
	vsnprintf(buffer, LARGE_STRING_SIZE, fmt, va);
	return(m_task->print(channel, thing, buffer));
}

int MooThing::print(MooThing *channel, MooThing *thing, MooArgs *args, const char *fmt)
{
	char buffer[LARGE_STRING_SIZE];

	if (!m_task)
		return(-1);
	// TODO should you have this function with va? or should you have it without, or both?  Same for MooThing::expand_str
	MooThing::expand_str(buffer, LARGE_STRING_SIZE, args, fmt);
	return(m_task->print(channel, thing, buffer));
}
*/

int MooThing::printf(MooArgs *args, const char *fmt, ...)
{
	va_list va;
	char buffer2[LARGE_STRING_SIZE];

	{
		// TODO should you have this function with va? or should you have it without, or both?  Same for MooThing::expand_str
		char buffer1[LARGE_STRING_SIZE];
		va_start(va, fmt);
		vsnprintf(buffer1, LARGE_STRING_SIZE, fmt, va);
		MooThing::expand_str(buffer2, LARGE_STRING_SIZE, args, buffer1);
	}
	return(this->notify(TNT_STATUS, NULL, NULL, buffer2));
}


int MooThing::notify(int type, MooThing *channel, MooThing *thing, const char *text)
{
	// TODO permissions check!?!?
	// TODO this would call an action, but since MooUser overrides this virtual function, we will either do nothing if it's
	//	a MooThing, or we'll call the m_task notify function if it's a MooUser
	return(0);
}

int MooThing::notify_all(int type, MooThing *channel, MooThing *thing, const char *text)
{
	MooThing *cur;

	for (cur = this->contents(); cur; cur = cur->next()) {
		cur->notify(type, channel, thing, text);
	}
	return(0);
}

int MooThing::notify_all_except(MooThing *except, int type, MooThing *channel, MooThing *thing, const char *text)
{
	MooThing *cur;

	for (cur = this->contents(); cur; cur = cur->next()) {
		if (cur != except)
			cur->notify(type, channel, thing, text);
	}
	return(0);
}


int MooThing::cryolocker_store()
{
	MooThing *cryolocker;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	// TODO could this be dangerous if you had to create a new cryolocker object, and now it will move the thing even though
	//	it was already in the cryolocker, causing last_location to be erroneusly overwritten
	if (this->m_location != cryolocker) {
		this->set_property("last_location", m_location ? m_location->m_id : -1);
		// TODO call the action needed to notify that the object is leaving (so everyone in the room sees "Soandso leaves in a
		//	puff of smoke" or something like that

		// TODO how should the quit message thing work?  where will it come from (property?)
		if (this->m_location)
			this->m_location->notify_all(TNT_QUIT, NULL, this, "disappears in a puff of smoke.");
		cryolocker->add(this);
	}
	return(0);
}

int MooThing::cryolocker_revive()
{
	MooThingRef *ref;
	MooThing *cryolocker, *thing = NULL;

	if (!(cryolocker = MooThing::reference(MOO_CRYOLOCKER)))
		return(-1);
	if (this->m_location == cryolocker) {
		if ((ref = (MooThingRef *) this->get_property("last_location", &moo_thingref_obj_type)))
			thing = MooThing::lookup(ref->m_id);
		// TODO how the fuck does the 'by' param work?
		if (!thing || this->moveto(thing, NULL)) {
			if ((thing = MooThing::reference(MOO_START_ROOM)))
				this->moveto(thing, NULL);
		}
	}
	return(0);
}

/**
 * Arguments:
 *	this = thing to move
 *	thing = location to move to
 *	by = ????
 */
int MooThing::moveto(MooThing *thing, MooThing *by)
{
	// TODO perhaps this should really just be a do_action on the object which calls a moveto function
	// TODO fill this in
	// TODO this will check permissions of via to perform the action (??) and then
	//	call various actions on the objects to actually do the move

	// TODO this should be a setting or something in the user object
	if (this->m_location)
		// TODO these can't be here because this might be a teleport command...
		this->m_location->notify_all(TNT_LEAVE, NULL, this, "runs off in the distance.");
	thing->add(this);
	thing->notify_all(TNT_JOIN, NULL, this, "appears from through the mist.");
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
	if (moo_thing_table->set(id, this))
		m_id = id;
	else {
		m_id = -1;
		moo_status("Error: Attempted to reassign ID, %d", id);
	}
	return(m_id);
}

int MooThing::add(MooThing *obj)
{
	if (obj->m_location == this)
		return(0);
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

///// String Parsers /////

/**
 * Format a string using the given fmt string and place the resulting
 * string into the given buffer.  The number of characters written to
 * the buffer is returned.  If a $ is encountered, the substring up
 * to the next space is taken to be a reference name.  If the reference
 * name is enclosed in { }, then the reference name is take to be
 * up to the closing }.  The reference is evaluated into a string using
 * the MooThing::expand_reference function.  If a & follows the $ then the
 * resolved string is recursively expanded.
 */
int MooThing::expand_str(char *buffer, int max, MooArgs *args, const char *fmt)
{
	int i;
	int j = 0;

	max--;
	for (i = 0;(fmt[i] != '\0') && (j < max);i++) {
		if (fmt[i] == '\\') {
			if (fmt[++i] == '\0')
				break;
			i += MooThing::escape_char(&fmt[i], &buffer[j++]) - 1;
		}
		else if (fmt[i] == '$')
			j += MooThing::expand_reference(&buffer[j], max - j + 1, args, &fmt[i], &i);
		else
			buffer[j++] = fmt[i];
	}
	buffer[j] = '\0';
	return(j);
}

#define IS_NUMBER_CHAR(ch)	\
	( ((ch) >= 0x30) && ((ch) <= 0x39) )

#define IS_VALID_CHAR(ch)	\
	( (((ch) >= '0') && ((ch) <= '9'))	\
	|| (((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'z'))	\
	|| ((ch) == '.') || ((ch) == '_') )

/**
 * Expand the reference using the given args and copy the resulting string
 * to the given buffer up to the max number of characters.  The number
 * of characters written to the buffer is returned.  If the given str_count
 * pointer is not NULL then the number of characters used as the variable
 * name in str is added to the value it points to.  The given str may
 * start with the '$' char or may start just after it.
 */
int MooThing::expand_reference(char *buffer, int max, MooArgs *args, const char *str, int *used)
{
	int k;
	char delim;
	int recurse;
	int i = 0, j = 0;
	char value[STRING_SIZE];

	if (str[i] == '$')
		i++;

	if (str[i] == '&') {
		recurse = 1;
		i++;
	}
	else
		recurse = 0;

	if (str[i] == '{') {
		delim = '}';
		i++;
	}
	else
		delim = ' ';

	for (k = 0;(str[i] != '\0') && (str[i] != delim) && IS_VALID_CHAR(str[i]) && (k < max - 1);k++, i++)
		buffer[k] = str[i];
	buffer[k] = '\0';
	if (MooThing::resolve_reference(value, STRING_SIZE, args, buffer) >= 0) {
		if (recurse)
			j = MooThing::expand_str(buffer, max, args, value);
		else {
			strncpy(buffer, value, max - 1);
			if ((j = strlen(value)) >= max)
				j = max - 1;
		}
	}
	if (delim != '}')
		i--;

	if (used)
		*used += i;
	buffer[j] = '\0';
	return(j);
}

/**
 * Resolves the given reference using the given arguments and copies the
 * resulting string into the given buffer.  The number of characters written
 * to the buffer is returned.
 */
int MooThing::resolve_reference(char *buffer, int max, MooArgs *args, const char *ref)
{
	int i;
	char *name;
	MooThing *thing;
	MooObject *result = NULL;

	max--;
	if ((name = strchr(ref, '.'))) {
		i = name - ref;
		name++;
	}
	else
		i = strlen(ref);

	if (!strncmp(ref, "text", i)) {
		// TODO we are ignoring the rest of the reference for now ($text.prop <=> $text)
		strncpy(buffer, args->m_text, max);
		buffer[max] = '\0';
		return(strlen(args->m_text));
	}
	else if (!strncmp(ref, "action", i)) {
		// TODO we are ignoring the rest of the reference for now ($action.owner, $action.name)
		strncpy(buffer, args->m_action->name(), max);
		buffer[max] = '\0';
		return(strlen(buffer));
	}
	else if (!strncmp(ref, "user", i))
		thing = args->m_user;
	else if (!strncmp(ref, "this", i))
		thing = args->m_this;
	else if (!strncmp(ref, "caller", i))
		thing = args->m_caller;
	else if (!strncmp(ref, "object", i))
		thing = args->m_object;
	else if (!strncmp(ref, "target", i))
		thing = args->m_target;
	else if (!strncmp(ref, "result", i))
		result = args->m_result;
	else
		thing = MooThing::reference(ref);

	buffer[0] = '\0';
	if (name) {
		if (!thing || !(result = thing->get_property(name, NULL)))
			return(0);
	}
	else if (!result) {
		if ((i = snprintf(buffer, max, "#%d", thing->m_id)) < 0)
			return(0);
		buffer[i] = '\0';
		return(i);
	}

	if (result->is_a(&moo_string_obj_type)) {
		strncpy(buffer, ((MooString *) result)->m_str, max);
		return(((MooString *) result)->m_len);
	}
	else if (result->is_a(&moo_number_obj_type)) {
		if ((i = snprintf(buffer, max, "%f", ((MooNumber *) result)->m_num)) < 0)
			return(0);
		buffer[i] = '\0';
		return(i);
	}
	else if (result->is_a(&moo_thingref_obj_type)) {
		if ((i = snprintf(buffer, max, "#%d", ((MooThingRef *) result)->m_id)) < 0)
			return(0);
		buffer[i] = '\0';
		return(i);
	}
	// TODO should you check for the array type and print it??
	return(0);
}


/**
 * Convert the charcter escape sequence (assuming str starts after the escape
 * character) and stores the character in buffer[0].  The number of characters
 * read as a sequence from str is returned
 */
int MooThing::escape_char(const char *str, char *buffer)
{
	char number[3];

	if (*str == '\0')
		return(0);
	switch (str[0]) {
		case 'n':
			buffer[0] = '\n';
			break;
		case 'r':
			buffer[0] = '\r';
			break;
		case 't':
			buffer[0] = '\t';
			break;
		case 'e':
			buffer[0] = '\x1b';
			break;
		case 'x':
			if ((str[1] != '\0') && (str[2] != '\0')) {
				number[0] = str[1];
				number[1] = str[2];
				number[2] = '\0';
				buffer[0] = MooThing::atoi(number, 16);
			}
			return(3);
		default:
			if (IS_NUMBER_CHAR(str[0])) {
				buffer[0] = str[0] - 0x30;
				if (IS_NUMBER_CHAR(str[1])) {
					buffer[0] = (buffer[0] * 8) + str[1] - 0x30;
					if (IS_NUMBER_CHAR(str[2])) {
						buffer[0] = (buffer[0] * 8) + str[2] - 0x30;
						return(3);
					}
					return(2);
				}
				return(1);
			}
			else
				buffer[0] = str[0];
			break;
	}
	return(1);
}

/**
 * Convert a string of the given radix to an interger.
 */
int MooThing::atoi(const char *str, int radix)
{
	int i = -1, ret = 0, mul = 1;

	if (!str)
		return(0);

	while (str[++i] == ' ' || str[i] == '\t' || str[i] == '\n')
		if (str[i] == '\0')
			return(0);

	if (str[i] == '-') {
		mul = -1;
		i++;
	}

	for (;str[i] != '\0';i++) {
		ret *= radix;
		if (str[i] >= 0x30 && str[i] <= 0x39)
			ret += (str[i] - 0x30);
		else if (str[i] >= 0x41 && str[i] <= 0x5a)
			ret += (str[i] - 0x37);
		else if (str[i] >= 0x61 && str[i] <= 0x7a)
			ret += (str[i] - 0x57);
		else
			break;
	}
	return(ret * mul);
}

