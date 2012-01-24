/*
 * Object Name:	hash.cpp
 * Description:	Hash Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/code/code.h>

#include <sdm/objs/hash.h>

struct MooObjectType moo_hash_obj_type = {
	"hash",
	typeid(MooObjectHash).name(),
	(moo_type_load_t) load_moo_hash
};

static MooObjectHash *hash_methods = new MooObjectHash();
void moo_load_hash_methods(MooObjectHash *env);

int init_hash(void)
{
	moo_object_register_type(&moo_hash_obj_type);
	moo_load_hash_methods(hash_methods);
	return(0);
}

void release_hash(void)
{
	MOO_DECREF(hash_methods);
	moo_object_deregister_type(&moo_hash_obj_type);
}

MooObject *load_moo_hash(MooDataFile *data)
{
	int id;
	MooObject *obj;

	if (data->child_of_root()) {
		obj = new MooObjectHash();
		obj->read_data(data);
	}
	else {
		id = data->read_integer();
		if (!(obj = MooMutable::lookup(id)))
			return(&moo_nil);
	}
	return(obj);
}

MooObjectHash::MooObjectHash(MooObjectHash *parent, int size, int bits) : MooHash<MooObject *>(size, bits, (void (*)(MooObject *)) MooGC::decref)
{
	m_parent = parent;
}

int MooObjectHash::read_entry(const char *type, MooDataFile *data)
{
	moo_id_t owner;
	moo_mode_t mode;
	MooObject *obj = NULL;
	char key[STRING_SIZE];
	char type_name[STRING_SIZE];

	if (!strcmp(type, "entry")) {
		data->read_attrib_string("type", type_name, STRING_SIZE);
		data->read_attrib_string("key", key, STRING_SIZE);
		owner = data->read_attrib_integer("owner");
		mode = data->read_attrib_integer("mode");
		if ((obj = MooObject::read_object(data, type_name))) {
			moo_status("HASH: Error loading entry, %s", key);
			return(-1);
		}
		this->set(key, obj, owner, mode);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooObjectHash::write_object(MooDataFile *data)
{
	MooHashEntry<MooObject *> *cur;

	MooMutable::write_object(data);
	this->reset();
	while ((cur = this->next_entry())) {
		data->write_begin_entry("entry");
		data->write_attrib_string("key", cur->m_key);
		data->write_attrib_string("type", cur->m_data->objtype_name());
		data->write_attrib_integer("owner", cur->m_owner);
		data->write_attrib_octal("mode", cur->m_mode);
		cur->m_data->write_data(data);
		data->write_end_entry();
	}
	return(0);
}

int MooObjectHash::to_string(char *buffer, int max)
{

}

int MooObjectHash::mutate(const char *key, MooObject *obj)
{
	MooObjectHash *cur;

	for (cur = this; cur; cur = cur->m_parent) {
		if (cur->get_local(key)) {
			return(cur->set(key, obj));
		}
	}
	return(this->set(key, obj));
}

MooObject *MooObjectHash::get(const char *key)
{
	MooObject *obj;
	MooObjectHash *cur;

	for (cur = this; cur; cur = cur->m_parent) {
		if ((obj = cur->get_local(key))) {
			// TODO optionally cache the entry in 'this'
			return(obj);
		}
	}
	return(NULL);
}

MooObject *MooObjectHash::get_local(const char *key)
{
	return(MooHash<MooObject *>::get(key));
}

long int MooObjectHash::get_integer(const char *key)
{
	MooObject *obj;

	if (!(obj = this->get(key)))
		return(0);
	return(obj->get_integer());
}

double MooObjectHash::get_float(const char *key)
{
	MooObject *obj;

	if (!(obj = this->get(key)))
		return(0);
	return(obj->get_float());
}

const char *MooObjectHash::get_string(const char *key)
{
	MooObject *obj;

	if (!(obj = this->get(key)))
		return(0);
	return(obj->get_string());
}

MooThing *MooObjectHash::get_thing(const char *key)
{
	MooObject *obj;

	if (!(obj = this->get(key)))
		return(0);
	return(obj->get_thing());
}

MooObject *MooObjectHash::access_property(const char *name, MooObject *value)
{
	if (value) {
		if (this->set(name, value) < 0)
			return(NULL);
		return(value);
	}
	else
		return(this->get(name));
}

MooObject *MooObjectHash::access_method(const char *name, MooObject *value)
{
	if (value)
		throw moo_permissions;
	return(hash_methods->get(name));
}

/************************
 * Hash Object Methods *
 ************************/

static int hash_get(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	const char *name;
	MooObjectHash *m_this;

	// TODO all these functions will bypass permissions checks unless we put them in get/set
	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 1)
		throw moo_args_mismatched;
	name = args->get_string(1);
	frame->set_return(m_this->get(name));
	return(0);
}

static int hash_set(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	int res;
	MooObject *obj;
	const char *name;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 2)
		throw moo_args_mismatched;
	name = args->get_string(1);
	obj = args->get(2);
	res = m_this->set(name, obj);
	frame->set_return((res == 0) ? &moo_true : &moo_false);
	return(0);
}

static int hash_remove(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	int res;
	const char *name;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 1)
		throw moo_args_mismatched;
	name = args->get_string(1);
	res = m_this->remove(name);
	frame->set_return((res == 0) ? &moo_true : &moo_false);
	return(0);
}

static int hash_keys(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooObjectHash *m_this;
	MooObjectArray *array;
	MooHashEntry<MooObject *> *entry;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 0)
		throw moo_args_mismatched;
	array = new MooObjectArray();
	m_this->reset();
	while ((entry = m_this->next_entry()))
		array->push(new MooString("%s", entry->m_key));
	frame->set_return(array);
	return(0);
}

static int hash_to_array(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooObjectHash *m_this;
	MooObjectArray *array;
	MooHashEntry<MooObject *> *entry;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 0)
		throw moo_args_mismatched;
	array = new MooObjectArray();
	m_this->reset();
	while ((entry = m_this->next_entry()))
		array->push(MOO_INCREF(entry->m_data));
	frame->set_return(array);
	return(0);
}

class HashEventForeach : public MooCodeEvent {
	int m_index;
	MooHashEntry<MooObject *> *m_next;
	MooObjectHash *m_this;
	MooObject *m_func;
	
    public:
	HashEventForeach(int index, MooHashEntry<MooObject *> *next, MooObjectHash *obj, MooObjectHash *env, MooObject *func) : MooCodeEvent(env, NULL, NULL) {
		m_index = index;
		m_next = next;
		// TODO incref??
		m_this = obj;
		m_func = func;
	}

	int do_event(MooCodeFrame *frame) {
		MooObject *obj;
		MooObjectArray *args;
		MooHashEntry<MooObject *> *entry;

		// TODO this is disabled for now because it causes trouble if you don't pay attention to the return type
		/// Stop this loop if the previous cycle returned #f
		//if ((obj = frame->get_return()) && !obj->is_true())
		//	return(0);
		if (!(entry = m_next)) {
			for (; m_index < m_this->m_size; m_index++) {
				if ((entry = m_this->m_table[m_index]))
					break;
			}
		}
		if (!entry)
			return(0);

		if (m_index < m_this->m_size) {
			if (entry->m_next)
				frame->push_event(new HashEventForeach(m_index, entry->m_next, m_this, m_env, m_func));
			else
				frame->push_event(new HashEventForeach(m_index + 1, NULL, m_this, m_env, m_func));
		}

		args = new MooObjectArray();
		args->set(0, MOO_INCREF(m_func));
		args->set(1, MOO_INCREF(entry->m_data));
		frame->push_event(new MooCodeEventCallFunc(m_env, args));
		return(0);
	}
};

static int hash_foreach(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	MooObject *func;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->get(0))))
		throw moo_method_object;
	if (args->last() != 1)
		throw moo_args_mismatched;
	func = args->get(1);
	frame->set_return(&moo_true);
	frame->push_event(new HashEventForeach(0, NULL, m_this, env, func));
	return(0);
}

void moo_load_hash_methods(MooObjectHash *env)
{
	env->set("get", new MooFunc(hash_get));
	env->set("set", new MooFunc(hash_set));
	env->set("remove", new MooFunc(hash_remove));
	env->set("keys", new MooFunc(hash_keys));
	env->set("->array", new MooFunc(hash_to_array));
	env->set("foreach", new MooFunc(hash_foreach));
}


