/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/nil.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/mutable.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/funcptr.h>
#include <sdm/objs/thing.h>

#include <sdm/code/code.h>


MooObjectType moo_thing_obj_type = {
	"thing",
	typeid(MooThing).name(),
	(moo_type_load_t) load_moo_thing
};

static MooObjectHash *thing_methods = new MooObjectHash();
void moo_load_thing_methods(MooObjectHash *env);

int init_thing(void)
{
	moo_object_register_type(&moo_thing_obj_type);
	moo_load_thing_methods(thing_methods);
	return(0);
}

void release_thing(void)
{
	moo_object_deregister_type(&moo_thing_obj_type);
}

MooObject *load_moo_thing(MooDataFile *data)
{
	int id;
	MooObject *obj;

	if (data->child_of_root()) {
		obj = new MooThing();
		obj->read_data(data);
	}
	else {
		id = data->read_integer();
		if (!(obj = MooMutable::lookup(id)))
			return(&moo_nil);
	}
	return(obj);
}

MooThing::MooThing(moo_id_t id, moo_id_t parent, moo_id_t owner, moo_mode_t mode)
{
	// TODO temporary since we haven't properly dealt with refcounting/thingrefs
	this->set_nofree();

	// TODO you should set these to be unassignable or something
	m_properties = new MooObjectHash();
	m_methods = new MooObjectHash();

	/// Set the thing id and add the thing to the table.  If id = MOO_NO_ID, don't add it to a table.
	/// If the id = MOO_NEW_ID then assign the next available id
	this->assign_id(id);
	m_parent = parent;
	m_owner = owner;
	m_mode = mode;
}

MooThing::~MooThing()
{
	/// When we set these to NULL, the garbage collector will free them
	m_properties = NULL;
	m_methods = NULL;
}

MooThing *MooThing::clone(moo_id_t id)
{
	MooThing *thing;

	if (MooMutable::is_assigned(id)) {
		moo_status("THING: ID %d is already assigned", id);
		return(NULL);
	}

	if (!(thing = new MooThing(id, this->id())))
		throw MooException("Error creating new thing from %d", this->id());
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
	else if (!strcmp(type, "parent")) {
		moo_id_t id = data->read_integer_entry();
		m_parent = id;
	}
	else if (!strcmp(type, "owner")) {
		moo_id_t owner = data->read_integer_entry();
		m_owner = owner;
	}
	else if (!strcmp(type, "mode")) {
		moo_mode_t mode = data->read_integer_entry();
		m_mode = mode;
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooThing::write_object(MooDataFile *data)
{
	MooMutable::write_object(data);
	if (m_parent >= 0)
		data->write_integer_entry("parent", m_parent);
	if (m_owner >= 0)
		data->write_integer_entry("owner", m_owner);
	if (m_mode != MOO_DEFAULT_MODE)
		data->write_integer_entry("mode", m_mode);

	/// Write the properties to the file
	if (m_properties->entries()) {
		data->write_begin_entry("properties");
		m_properties->write_object(data);
		data->write_end_entry();
	}

	/// Write the actions to the file
	if (m_methods->entries()) {
		data->write_begin_entry("methods");
		m_methods->write_object(data);
		data->write_end_entry();
	}
	return(0);
}

int MooThing::to_string(char *buffer, int max)
{
	return(snprintf(buffer, max, "#%d", this->id()));
}

MooObject *MooThing::access_property(const char *name, MooObject *value)
{
	MooHashEntry<MooObject *> *entry;

	if (!strcmp(name, "id"))
		return(value ? NULL : new MooNumber((long int) this->id()));
	else if (!strcmp(name, "parent"))
		return(value ? NULL : new MooNumber((long int) this->m_parent));
	else if (!strcmp(name, "owner"))
		return(value ? NULL : new MooNumber((long int) this->m_owner));
	else if (!strcmp(name, "mode"))
		return(value ? NULL : new MooNumber((long int) this->m_mode));

	if (value) {
		if (!name || (*name == '\0'))
			return(NULL);
		entry = m_properties->get_entry(name);

		if (entry) {
			this->check_throw(MOO_MODE_W, entry->m_owner, entry->m_mode);
			entry->m_data = MOO_INCREF(value);
		}
		else {
			this->check_throw(MOO_MODE_W, m_owner, m_mode);
			if (m_properties->set(name, MOO_INCREF(value), MooCodeFrame::current_owner(), MOO_DEFAULT_MODE))
				return(NULL);
		}
		return(value);
	}
	else {
		for (MooThing *thing = this; thing; thing = thing->parent()) {
			// TODO you could add some kind of pub/private/protected/whatever permissions checks here
			if ((entry = thing->m_properties->get_entry(name)))
				break;
		}
		// TODO should you return nil if obj == NULL (??)
		if (!entry)
			return(NULL);
		this->check_throw(MOO_MODE_R, entry->m_owner, entry->m_mode);
		return(entry->m_data);
	}
}

MooObject *MooThing::access_method(const char *name, MooObject *value)
{
	MooHashEntry<MooObject *> *entry;

	if (value) {
		if (!name || (*name == '\0'))
			return(NULL);
		/// If the new value is nil, remove the entry from the table
		/// 	(It doesn't even make sense for a method to be nil since it would case an exception, so it doesn't matter if we remove it)
		if (MOO_IS_NIL(value)) {
			if (m_methods->remove(name))
				return(NULL);
			return(value);
		}

		entry = m_methods->get_entry(name);
		if (entry) {
			this->check_throw(MOO_MODE_W, entry->m_owner, entry->m_mode);
			entry->m_data = value;
		}
		else {
			this->check_throw(MOO_MODE_W, m_owner, m_mode);
			if (m_methods->set(name, MOO_INCREF(value), MooCodeFrame::current_owner(), MOO_DEFAULT_MODE))
				return(NULL);
		}
		return(value);
	}
	else {
		MooThing *cur;

		if ((entry = thing_methods->get_entry(name))) {
			this->check_throw(MOO_MODE_R, entry->m_owner, entry->m_mode);
			return(entry->m_data);
		}

		for (cur = this; cur; cur = cur->parent()) {
			if ((entry = cur->m_methods->get_entry(name))) {
				this->check_throw(MOO_MODE_R, entry->m_owner, entry->m_mode);
				return(entry->m_data);
			}
		}
		return(NULL);
	}
}


void MooThing::check_throw(moo_mode_t check, moo_id_t owner, moo_mode_t mode)
{
	if (!this->check(check, owner, mode))
		throw moo_permissions;
}

int MooThing::check(moo_mode_t check, moo_id_t owner, moo_mode_t mode)
{
	moo_id_t current;

	current = MooCodeFrame::current_owner();
	if (MooThing::is_wizard(current))
		return(1);
	if (owner == current)
		check <<= 3;
	if ((mode & check) == check)
		return(1);
	return(0);
}

const char *MooThing::name()
{
	MooObject *obj;
	const char *name = NULL;

	// TODO get rid of this whole function (or at least move it to funcs/ somewhere, it doesn't belong here)
	try {
		if (!(obj = this->resolve_property("name", NULL)))
			return(NULL);
		name = obj->get_string();
	}
	// TODO get rid of this especially
	catch (...) { }

	if (!name)
		return("???");
	return(name);
}

MooThing *MooThing::get_channel(const char *name)
{
	MooObject *channels;
	MooObjectHash *list;

	if (!(channels = MooObject::resolve("ChanServ", global_env))) {
		moo_status("CHANNEL: ChanServ object not found");
		return(NULL);
	}
	if (!(list = dynamic_cast<MooObjectHash *>(channels->resolve_property("db")))) {
		moo_status("CHANNEL: channel list not found in ChanServ");
		return(NULL);
	}
	return(dynamic_cast<MooThing *>(list->get(name)));
}

void MooThing::quit()
{
	MooObject *channels;

	if ((channels = MooObject::resolve("ChanServ", global_env)))
		channels->call_method(NULL, "quit", NULL);
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


/************************
 * Thing Object Methods *
 ************************/

static int thing_clone(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooObject *func;
	MooObject *id = NULL;
	MooObjectArray *newargs;
	MooThing *thing, *parent;
	moo_id_t idnum = MOO_NEW_ID;

	if (!(parent = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (args->last() == 1)
		func = args->get(1);
	else if (args->last() == 2) {
		id = args->get(1);
		func = args->get(2);
	}
	else
		throw moo_args_mismatched;
	if (id)
		idnum = id->get_integer();

	// TODO permissions check!!!
	thing = parent->clone(idnum);
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), new MooCodeExpr(0, 0, MCT_OBJECT, thing, NULL)));
	if (func) {
		newargs = new MooObjectArray();
		newargs->set(0, thing);
		frame->push_call(env, func, newargs);
	}

	/// Call the 'initialize' method of each parent object (Most distant parent will be called first)
	while (parent) {
		if ((func = parent->resolve_method("initialize"))) {
			newargs = new MooObjectArray();
			newargs->set(0, thing);
			frame->push_call(env, func, newargs);
		}
		parent = parent->parent();
	}
	return(0);
}

static int thing_load(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 0)
		throw moo_args_mismatched;
	//  TODO FIX THIS
	//thing->load();
	return(0);
}

static int thing_save(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 0)
		throw moo_args_mismatched;
	// TODO FIX THIS
	//thing->save();
	return(0);
}

static int thing_save_all(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	// TODO permissions check
	MooThing::save_all();
	return(0);
}

static int thing_owner(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooThing *thing;
	const char *str;

/*
	if (args->last() == 1)
		str = args->get_string(1);
	else if (args->last() != 0)
		throw moo_args_mismatched;
	if (!(thing = args->get_thing(0)))
		throw moo_type_error;

	if (!str)
		frame->set_return(new MooNumber((long int) thing->owner()));
	else {
		if (str[0] == ':')
			entry = thing->m_methods->get_entry(&str[1]);
		else
			entry = thing->m_properties->get_entry(str);
		frame->set_return(new MooNumber((long int) entry->m_owner));
	}
*/
	return(0);
}

static int thing_mode(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooObject *obj;
	// TODO FIX THIS
/*
	if (args->last() <= 0)
		throw moo_args_mismatched;
	if (!(obj = args->get(0)))
		frame->set_return(new MooNumber((long int) m_owner));
	else {
		// TODO you need to fix this but you have both properties and methods to deal with
		//frame->set_return(new MooNumber((long int) obj->owner()));
	}
*/
	return(0);
}

static int thing_chmod(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	// TODO FIX THIS
/*
	MooThing *owner = NULL;
	MooObject *obj, *perms;

	if (args->last() == 1)
		obj = args->get(1);
	else if (args->last() == 2) {
		if ((obj = args->get(1)) && !(owner = dynamic_cast<MooThing *>(obj)))
			throw moo_type_error;
		obj = args->get(2);
	}
	else
		throw moo_args_mismatched;

	obj->check_throw(MOO_MODE_W);
	if ((perms = args->get(0)))
		obj->permissions(perms->get_integer());
	if (owner)
		obj->owner(owner->id());
	frame->set_return(obj);
*/
	return(0);
}

static int thing_chown(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	// TODO FIX THIS
/*
	MooThing *owner = NULL;
	MooObject *obj, *perms;

	if (args->last() == 1)
		obj = args->get(1);
	else if (args->last() == 2) {
		if ((obj = args->get(1)) && !(owner = dynamic_cast<MooThing *>(obj)))
			throw moo_type_error;
		obj = args->get(2);
	}
	else
		throw moo_args_mismatched;

	obj->check_throw(MOO_MODE_W);
	if ((perms = args->get(0)))
		obj->permissions(perms->get_integer());
	if (owner)
		obj->owner(owner->id());
	frame->set_return(obj);
*/
	return(0);
}

/*
#define MAX_WORDS	256
#define PREPOSITIONS	5
const char *prepositions[] = { "to", "in", "from", "is", "as" };

static int parse_command(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int k, m;
	int i = 0, j = 0;
	MooObject *method, *user;
	const char *text, *cmd, *argstr;
	char *words[MAX_WORDS];
	MooArgs *newargs;
	MooObjectHash *newenv;
	char buffer[LARGE_STRING_SIZE];

	if (!(user = env->get("user")))
		throw MooException("No user object set.");
	text = args->m_args->get_string(0);
	argstr = parser_next_word(text);

	// Parse the text into words
	strncpy(buffer, text, LARGE_STRING_SIZE);
	buffer[LARGE_STRING_SIZE] = '\0';
	while (parser_is_whitespace(buffer[i]))
		i++;
	words[0] = &buffer[i];
	for (; buffer[i] != '\0'; i++) {
		if (buffer[i] == '\"') {
			words[j] = &buffer[++i];
			for (; buffer[i] != '\0' && buffer[i] != '\"'; i++)
				;
		}

		if (buffer[i] == '\"' || parser_is_whitespace(buffer[i])) {
			buffer[i++] = '\0';
			while (parser_is_whitespace(buffer[i]))
				i++;
			words[++j] = &buffer[i];
		}
	}

	/// Build the arguments
	newargs = new MooArgs();
	newenv = new MooObjectHash(env);
	cmd = words[0];
	newenv->set("argstr", new MooString("%s", argstr));
	for (k = 1; k < j; k++)
		newargs->m_args->push(new MooString("%s", words[k]));

	for (k = 1; k < j; k++) {
		for (m = 0; m < PREPOSITIONS; m++)
			if (!strcasecmp(words[k], prepositions[m]))
				break;
	}
	newenv->set("prep", new MooString("%s", (k < j) ? words[k++] : ""));

	if (!(method = user->resolve_method(cmd))) {
		MooObject *location = user->resolve_property("location");
		if (location && !(method = location->resolve_method(cmd))) {
			// TODO try to parse more and search the objects
		}
	}

	frame->push_debug("> in realm_command: %s", cmd);
	frame->push_call(newenv, method, newargs);

	// TODO you could have a call here to an optional method on the user after a command has been executed (like a prompt)

	return(0);
}
*/

void moo_load_thing_methods(MooObjectHash *env)
{
	// TODO this is wrong because it will *effectively* add these methods to all things without setting permissions for them
	// 	which i *think* will make them all (specifically %save_all) executable by anyone
	env->set("%clone", new MooFuncPtr(thing_clone));
	env->set("%load", new MooFuncPtr(thing_load));
	env->set("%save", new MooFuncPtr(thing_save));
	env->set("%save_all", new MooFuncPtr(thing_save_all));
	env->set("owner", new MooFuncPtr(thing_owner));
	env->set("mode", new MooFuncPtr(thing_mode));
	env->set("chmod", new MooFuncPtr(thing_chmod));
	env->set("chown", new MooFuncPtr(thing_chown));
	//env->set("%command", new MooFuncPtr(parse_command));
}


