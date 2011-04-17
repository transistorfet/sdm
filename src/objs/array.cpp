/*
 * Object Name:	array.cpp
 * Description:	Array Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/code/code.h>

#include <sdm/things/thing.h>
#include <sdm/array.h>

struct MooObjectType moo_array_obj_type = {
	NULL,
	"array",
	typeid(MooObjectArray).name(),
	(moo_type_create_t) moo_array_create
};

static MooObjectHash *array_methods = new MooObjectHash();
void moo_load_array_methods(MooObjectHash *env);

int init_array(void)
{
	moo_object_register_type(&moo_array_obj_type);
	moo_load_array_methods(array_methods);
	return(0);
}

void release_array(void)
{
	MOO_DECREF(array_methods);
	moo_object_deregister_type(&moo_array_obj_type);
}

MooObject *moo_array_create(void)
{
	return(new MooObjectArray(MOO_ARRAY_DEFAULT_SIZE, -1, MOO_OBJECT_ARRAY_DEFAULT_BITS));
}

MooObjectArray::MooObjectArray(int size, int max, int bits) : MooArray<MooObject *>(size, max, bits, (void (*)(MooObject *)) MooGC::decref)
{
	/// Nothing to be done.  This is just here for the call to the MooArray constructor
}

int MooObjectArray::read_entry(const char *type, MooDataFile *data)
{
	int res, index;
	MooObject *obj = NULL;
	char buffer[STRING_SIZE];
	const MooObjectType *objtype;

	// TODO read/write the other values?? like max?
	if (!strcmp(type, "entry")) {
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, NULL))) {
			moo_status("ARRAY: Unable to find entry type, %s", buffer);
			return(-1);
		}
		index = data->read_attrib_integer("index");
		if (!(obj = moo_make_object(objtype))) {
			moo_status("ARRAY: Error loading entry, %d", index);
			return(-1);
		}
		data->read_children();
		res = obj->read_data(data);
		data->read_parent();
		if ((res < 0) || (this->set(index, obj) < 0)) {
			MOO_DECREF(obj);
			return(-1);
		}
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooObjectArray::write_data(MooDataFile *data)
{
	MooObject::write_data(data);
	for (int i = 0; i < m_size; i++) {
		if (m_data[i]) {
			data->write_begin_entry("entry");
			data->write_attrib_integer("index", i);
			data->write_attrib_string("type", m_data[i]->type_name());
			m_data[i]->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

int MooObjectArray::to_string(char *buffer, int max)
{

}

long int MooObjectArray::get_integer(int index)
{
	MooObject *obj;

	if (!(obj = this->get(index)))
		return(0);
	return(obj->get_integer());
}

double MooObjectArray::get_float(int index)
{
	MooObject *obj;

	if (!(obj = this->get(index)))
		return(0);
	return(obj->get_float());
}

const char *MooObjectArray::get_string(int index)
{
	MooObject *obj;

	if (!(obj = this->get(index)))
		return(0);
	return(obj->get_string());
}

MooThing *MooObjectArray::get_thing(int index)
{
	MooObject *obj;

	if (!(obj = this->get(index)))
		return(0);
	return(obj->get_thing());
}

MooObject *MooObjectArray::access_property(const char *name, MooObject *value)
{
	if (value)
		throw moo_permissions;
	else if (!strcmp(name, "size"))
		return(new MooNumber((long int) m_size));
	else if (!strcmp(name, "last"))
		return(new MooNumber((long int) m_last));
	// TODO should you throw not-found?
	return(NULL);
}

MooObject *MooObjectArray::access_method(const char *name, MooObject *value)
{
	if (value)
		throw moo_permissions;
	// TODO do you need to do a read permissions check here?
	return(array_methods->get(name));
}

/************************
 * Array Object Methods *
 ************************/

static int array_get(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	long int index;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	index = args->m_args->get_integer(0);
	args->m_result = m_this->get(index);
	return(0);
}

static int array_set(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	long int index;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 1)
		throw moo_args_mismatched;
	index = args->m_args->get_integer(0);
	obj = args->m_args->get(1);
	args->m_result = m_this->set(index, obj);
	return(0);
}

static int array_remove(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int res;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	res = m_this->remove(obj);
	args->m_result = new MooNumber((long int) res);
	return(0);
}

static int array_push(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int res;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	res = m_this->push(obj);
	args->m_result = new MooNumber((long int) res);
	return(0);
}

static int array_pop(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() >= 0)
		throw moo_args_mismatched;
	args->m_result = m_this->pop();
	return(0);
}

static int array_shift(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() >= 0)
		throw moo_args_mismatched;
	args->m_result = m_this->shift();
	return(0);
}

static int array_unshift(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int res;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	res = m_this->unshift(obj);
	args->m_result = new MooNumber((long int) res);
	return(0);
}

static int array_insert(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int res;
	int index;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 1)
		throw moo_args_mismatched;
	index = args->m_args->get_integer(0);
	obj = args->m_args->get(1);
	res = m_this->insert(index, obj);
	args->m_result = new MooNumber((long int) res);
	return(0);
}

static int array_splice(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int index;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	index = args->m_args->get_integer(0);
	args->m_result = m_this->splice(index);
	return(0);
}


static int array_search(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	obj = args->m_args->get(0);
	for (int i = 0; i <= m_this->last(); i++) {
		if (m_this->get(i) == obj)
			args->m_result = new MooNumber((long int) i);
	}
	args->m_result = new MooNumber((long int) -1);
	return(0);
}

class ArrayEventForeach : public MooCodeEvent {
	int m_index;
	MooObjectArray *m_this;
	MooObject *m_func;
	
    public:
	ArrayEventForeach(int index, MooObjectArray *obj, MooObjectHash *env, MooObject *func) : MooCodeEvent(env, NULL, NULL) {
		m_index = index;
		// TODO incref??
		m_this = obj;
		m_func = func;
	}

	int do_event(MooCodeFrame *frame) {
		MooArgs *args;

		if (m_index > m_this->last())
			return(0);
		if (m_index < m_this->last())
			frame->push_event(new ArrayEventForeach(m_index + 1, m_this, m_env, m_func));
		args = new MooArgs();
		args->m_args->set(0, MOO_INCREF(m_func));
		args->m_args->set(1, MOO_INCREF(m_this->get(m_index)));
		frame->push_event(new MooCodeEventCallExpr(m_env, args));
		return(0);
	}
};

static int array_foreach(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObject *func;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	func = args->m_args->get(0);
	frame->push_event(new ArrayEventForeach(0, m_this, env, func));
	return(0);
}

void moo_load_array_methods(MooObjectHash *env)
{
	env->set("get", new MooCodeFunc(array_get, "&all"));
	env->set("set", new MooCodeFunc(array_set, "&all"));
	env->set("remove", new MooCodeFunc(array_remove, "&all"));
	env->set("push", new MooCodeFunc(array_push, "&all"));
	env->set("pop", new MooCodeFunc(array_pop, "&all"));
	env->set("shift", new MooCodeFunc(array_shift, "&all"));
	env->set("unshift", new MooCodeFunc(array_unshift, "&all"));
	env->set("insert", new MooCodeFunc(array_insert, "&all"));
	env->set("splice", new MooCodeFunc(array_splice, "&all"));

	env->set("search", new MooCodeFunc(array_search, "&all"));
	env->set("foreach", new MooCodeFunc(array_foreach, "&all"));
}

