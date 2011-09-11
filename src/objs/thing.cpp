/*
 * Object Name:	thing.c
 * Description:	Base Game Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/tasks/task.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/args.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/funcs/func.h>
#include <sdm/funcs/method.h>
#include <sdm/objs/thing.h>


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

static MooObjectHash *thing_methods = new MooObjectHash();
void moo_load_thing_methods(MooObjectHash *env);

int init_thing(void)
{
	if (moo_thing_table)
		return(1);
	moo_object_register_type(&moo_thing_obj_type);
	moo_thing_table = new MooArray<MooThing *>(MOO_THING_INIT_SIZE, MOO_THING_MAX_SIZE, THING_TABLE_BITS);
	moo_load_thing_methods(thing_methods);
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
	m_id = -1;
	this->assign_id(id);
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
	char buffer[STRING_SIZE];

	if (id < 0)
		return(NULL);
	if (!(thing = moo_thing_table->get(id))) {
		snprintf(buffer, STRING_SIZE, "objs/%04d.xml", id);
		if (!moo_data_file_exists(buffer))
			return(NULL);
		thing = new MooThing(id);
		if (thing->load() < 0)
			MOO_DECREF(thing);
	}
	return(thing);
}

MooThing *MooThing::clone(moo_id_t id)
{
	MooThing *thing;
	MooHashEntry<MooObject *> *entry;

	if ((thing = moo_thing_table->get(id))) {
		moo_status("WARNING: Reassigning thing ID, #%d", id);
		//thing->m_bits = 0;
		thing->m_parent = this->m_id;
		thing->m_properties->clear();
		thing->m_methods->clear();
	}
	else if (!(thing = new MooThing(id, this->m_id)))
		throw MooException("Error creating new thing from %d", this->m_id);

	/*
	this->m_properties->reset();
	while ((entry = this->m_properties->next_entry()))
		thing->m_properties->set(entry->m_key, MOO_INCREF(entry->m_data));
	*/
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
		m_bits = (bits & ~MOO_TBF_WRITING);
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
	// TODO you should either save this object (if not currently being saved) or queue it up or something, or
	//	perhaps you can have some kind of 'modified/dirty' bit (which would prevent repeated or unnecessary saving (but this
	//	wouldn't necessarily work because a subvalue could be modified and we woludn't save it)
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

	/// If we are currently writing the file to disc then don't write it again
	if (m_bits & MOO_TBF_WRITING)
		return(0);

	if (m_id < 0) {
		moo_status("THING: attempting to write thing with an invalid ID");
		return(-1);
	}
	//snprintf(file, STRING_SIZE, "objs/%04d/%04d.xml", m_id / 10000, m_id % 10000);
	snprintf(file, STRING_SIZE, "objs/%04d.xml", m_id);
	moo_status("Writing thing data to file \"%s\".", file);
	data = new MooDataFile(file, MOO_DATA_WRITE, "thing");

	m_bits |= MOO_TBF_WRITING;
	try {
		data->write_integer_entry("id", m_id);
		if (m_parent >= 0)
			data->write_integer_entry("parent", m_parent);
		if (m_bits & ~MOO_TBF_WRITING)
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
	}
	catch (MooException e) {
		m_bits &= ~MOO_TBF_WRITING;
		throw e;
	}
	m_bits &= ~MOO_TBF_WRITING;

	delete data;
	return(0);
}

int MooThing::save_all()
{
	MooThing *thing;

	for (int i = 0; i <= moo_thing_table->last(); i++) {
		thing = moo_thing_table->get(i);
		if (!thing)
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
	MooObject *cur;

	if (!strcmp(name, "id"))
		return(value ? NULL : new MooNumber((long int) this->m_id));
	else if (!strcmp(name, "parent"))
		return(value ? NULL : new MooNumber((long int) this->m_parent));

	if (value) {
		if (!name || (*name == '\0'))
			return(NULL);
		cur = m_properties->get(name);

		if (cur) {
			cur->check_throw(MOO_PERM_W);
			value->match_perms(cur);
		}
		else
			this->check_throw(MOO_PERM_W);
		if (m_properties->set(name, MOO_INCREF(value)))
			return(NULL);
		return(value);
	}
	else {
		for (MooThing *thing = this; thing; thing = thing->parent()) {
			// TODO you could add some kind of pub/private/protected/whatever permissions checks here
			if ((cur = thing->m_properties->get(name)))
				break;
		}
		if (cur)
			cur->check_throw(MOO_PERM_R);
		// TODO should you return nil if obj == NULL (??)
		return(cur);
	}
}

MooObject *MooThing::access_method(const char *name, MooObject *value)
{
	MooObject *obj;

	// TODO do you need to do permissions checks here?
	//this->check_throw(MOO_PERM_R);
	if (value) {
		if (this->set_method(name, value) < 0)
			return(NULL);
		return(value);
	}
	else {
		if ((obj = thing_methods->get(name)))
			return(obj);
		return(this->get_method(name));
	}
}

///// Method Methods /////

int MooThing::set_method(const char *name, MooObject *value)
{
	MooObject *cur;

	if (!name || (*name == '\0'))
		return(-1);

	if ((cur = m_methods->get(name)))
		cur->check_throw(MOO_PERM_W);
	else
		this->check_throw(MOO_PERM_W);

	/// If the new value is nil, remove the entry from the table
	/// 	(It doesn't even make sense for a method to be nil since it would case an exception, so it doesn't matter if we remove it)
	if (value && MOO_IS_NIL(value))
		return(m_methods->remove(name));
	return(m_methods->set(name, MOO_INCREF(value)));
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
		if (id < 0 || id > MOO_THING_MAX_SIZE)
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
	if (id == MOO_NEW_ID) {
		// TODO this will load every object in to the system, we need no change this if we want to only load the needed objs
		id = moo_thing_table->next_space();
		for (; id < MOO_THING_MAX_SIZE; id++) {
			if (!MooThing::lookup(id))
				break;
		}
	}

	if (id >= MOO_THING_MAX_SIZE)
		throw MooException("THING: Maximum number of things reached; assignment failed.");
	if (moo_thing_table->set(id, this))
		m_id = id;
	if (m_id < 0 && id != MOO_NO_ID)
		moo_status("Error: Attempted to reassign ID, %d", id);
	return(m_id);
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

int MooThing::connect(MooTask *task)
{
	int res;
	MooArgs *args;
	MooObject *func;

	if (!(func = this->resolve_method("connect")))
		return(-1);
	args = new MooArgs();
	args->m_args->set(0, task);
	res = this->call_method(NULL, func, args);
	MOO_DECREF(args);
	return(res);
}

void MooThing::disconnect()
{
	this->call_method(NULL, "disconnect", NULL);
}

int MooThing::notify(int type, MooThing *thing, MooThing *channel, const char *text)
{
	int res;
	MooArgs *args;
	MooObject *func;

	if (!(func = this->resolve_method("notify")))
		return(-1);
	args = new MooArgs();
	args->m_args->set(0, new MooNumber((long int) type));
	args->m_args->set(1, thing);
	args->m_args->set(2, channel);
	args->m_args->set(3, new MooString("%s", text));
	res = this->call_method(channel, func, args);
	MOO_DECREF(args);
	return(res);
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

static int thing_clone(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooMethod *init;
	MooObject *func;
	MooObject *id = NULL;
	MooThing *thing, *parent;
	moo_id_t idnum = MOO_NEW_ID;

	if (!(parent = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() == 0)
		func = args->m_args->get(0);
	else if (args->m_args->last() == 1) {
		id = args->m_args->get(0);
		func = args->m_args->get(1);
	}
	else
		throw moo_args_mismatched;
	if (id)
		idnum = id->get_integer();

	// TODO permissions check!!!
	thing = parent->clone(idnum);
	frame->push_event(new MooCodeEventEvalExpr(frame->env(), new MooCodeExpr(0, 0, MCT_OBJECT, thing, NULL)));
	if (func)
		frame->push_call(env, new MooMethod(thing, func), new MooArgs());

	/// Call the 'initialize' method of each parent object (Most distant parent will be called first)
	while (parent) {
		if ((init = dynamic_cast<MooMethod *>(parent->resolve_method("initialize")))) {
			init->m_obj = thing;
			frame->push_call(env, init, new MooArgs());
		}
		parent = parent->parent();
	}
	return(0);
}

static int thing_load(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != -1)
		throw moo_args_mismatched;
	thing->load();
	return(0);
}

static int thing_save(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *thing;

	// TODO permissions check
	if (!(thing = dynamic_cast<MooThing *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != -1)
		throw moo_args_mismatched;
	thing->save();
	return(0);
}

static int thing_save_all(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO permissions check
	MooThing::save_all();
	return(0);
}

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

/*
	(define this:command (lambda (text)
		(define words (***parse-into-words***)
		(define dobj "")
		(define prep "")
		(define iobj "")
		(words:foreach (lambda (cur)
			(cond
				((!= (prepositions:search cur) -1)
					(set! prep cur))
				((equal? prep "")
					(set! dobj (concat dobj " " cur)))
				(else
					(set! iobj (concat iobj " " cur)))
		))
	))
*/


}

void moo_load_thing_methods(MooObjectHash *env)
{
	env->set("%clone", new MooFunc(thing_clone));
	env->set("%load", new MooFunc(thing_load));
	env->set("%save", new MooFunc(thing_save));
	env->set("%save_all", new MooFunc(thing_save_all));
	env->set("%command", new MooFunc(parse_command));
}


