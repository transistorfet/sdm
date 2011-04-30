/*
 * Object Name:	hash.cpp
 * Description:	Hash Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/code/code.h>

#include <sdm/things/thing.h>
#include <sdm/objs/hash.h>

struct MooObjectType moo_hash_obj_type = {
	NULL,
	"hash",
	typeid(MooObjectHash).name(),
	(moo_type_make_t) make_moo_hash
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

MooObject *make_moo_hash(MooDataFile *data)
{
	MooObjectHash *obj = new MooObjectHash();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooObjectHash::MooObjectHash(MooObjectHash *parent, int size, int bits) : MooHash<MooObject *>(size, bits, (void (*)(MooObject *)) MooGC::decref)
{
	m_parent = parent;
}

int MooObjectHash::read_entry(const char *type, MooDataFile *data)
{
	int res;
	MooObject *obj = NULL;
	char key[STRING_SIZE];
	char type_name[STRING_SIZE];
	const MooObjectType *objtype;

	if (!strcmp(type, "entry")) {
		data->read_attrib_string("type", type_name, STRING_SIZE);
		if (!(objtype = moo_object_find_type(type_name, NULL))) {
			moo_status("HASH: Unable to find entry type, %s", type_name);
			return(-1);
		}
		data->read_attrib_string("key", key, STRING_SIZE);
		res = data->read_children();
		if (!(obj = moo_make_object(objtype, res ? data : NULL))) {
			moo_status("HASH: Error loading entry, %s", key);
			return(-1);
		}
		if (res)
			data->read_parent();
		this->set(key, obj);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooObjectHash::write_data(MooDataFile *data)
{
	MooHashEntry<MooObject *> *cur;

	MooObject::write_data(data);
	this->reset();
	while ((cur = this->next_entry())) {
		data->write_begin_entry("entry");
		data->write_attrib_string("key", cur->m_key);
		data->write_attrib_string("type", cur->m_data->type_name());
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
		if ((obj = cur->get_local(key))) {
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
	// TODO do you need to do permissions checks here?
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
	// TODO do you need to do a read permissions check here?
	return(hash_methods->get(name));
}

/************************
 * Hash Object Methods *
 ************************/

static int hash_get(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *name;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	name = args->m_args->get_string(0);
	args->m_result = m_this->get(name);
	return(0);
}

static int hash_set(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	const char *name;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 1)
		throw moo_args_mismatched;
	name = args->m_args->get_string(0);
	obj = args->m_args->get(1);
	m_this->set(name, obj);
	return(0);
}

static int hash_remove(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	const char *name;
	MooObjectHash *m_this;

	if (!(m_this = dynamic_cast<MooObjectHash *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	name = args->m_args->get_string(0);
	m_this->remove(name);
	return(0);
}

void moo_load_hash_methods(MooObjectHash *env)
{
	env->set("get", new MooFunc(hash_get));
	env->set("set", new MooFunc(hash_set));
	env->set("remove", new MooFunc(hash_remove));
}


