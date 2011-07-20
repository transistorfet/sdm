/*
 * Object Name:	array.cpp
 * Description:	Array Object
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/code/code.h>

#include <sdm/objs/array.h>

struct MooObjectType moo_array_obj_type = {
	NULL,
	"array",
	typeid(MooObjectArray).name(),
	(moo_type_make_t) make_moo_array
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

MooObject *make_moo_array(MooDataFile *data)
{
	MooObjectArray *obj = new MooObjectArray();
	if (data)
		obj->read_data(data);
	return(obj);
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
		res = data->read_children();
		if (!(obj = moo_make_object(objtype, res ? data : NULL))) {
			moo_status("ARRAY: Error loading entry, %d", index);
			return(-1);
		}
		if (res)
			data->read_parent();
		this->set(index, obj);
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
			data->write_attrib_string("type", m_data[i]->objtype_name());
			m_data[i]->write_data(data);
			data->write_end_entry();
		}
	}
	return(0);
}

int MooObjectArray::to_string(char *buffer, int max)
{
	int j = 0;
	MooObject *obj;

	if (this->last() >= 0) {
		obj = this->get(0);
		j += obj->to_string(&buffer[j], max - j);
		for (int i = 1; i <= this->last() && j < max; i++) {
			buffer[j++] = ' ';
			obj = this->get(i);
			j += obj->to_string(&buffer[j], max - j);
		}
	}
	buffer[j] = '\0';
	return(j);
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
	this->check_throw(MOO_PERM_R);

	if (!strcmp(name, "size"))
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
	this->check_throw(MOO_PERM_R);
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
	m_this->check_throw(MOO_PERM_R);
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
	m_this->check_throw(MOO_PERM_W);
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
	m_this->check_throw(MOO_PERM_W);
	obj = args->m_args->get(0);
	res = m_this->remove(obj);
	args->m_result = new MooBoolean(res != 0);
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
	m_this->check_throw(MOO_PERM_W);
	obj = args->m_args->get(0);
	res = m_this->push(obj);
	args->m_result = new MooBoolean(res == 0);
	return(0);
}

static int array_pop(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() >= 0)
		throw moo_args_mismatched;
	m_this->check_throw(MOO_PERM_W);
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
	m_this->check_throw(MOO_PERM_W);
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
	m_this->check_throw(MOO_PERM_W);
	obj = args->m_args->get(0);
	res = m_this->unshift(obj);
	args->m_result = new MooBoolean(res == 0);
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
	m_this->check_throw(MOO_PERM_W);
	index = args->m_args->get_integer(0);
	obj = args->m_args->get(1);
	res = m_this->insert(index, obj);
	args->m_result = new MooBoolean(res == 0);
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
	m_this->check_throw(MOO_PERM_W);
	index = args->m_args->get_integer(0);
	args->m_result = m_this->splice(index);
	return(0);
}


static int array_search(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int index;
	MooObject *obj;
	MooObjectArray *m_this;

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	m_this->check_throw(MOO_PERM_R);
	obj = args->m_args->get(0);
	index = m_this->search(obj);
	args->m_result = new MooNumber((long int) index);
	return(0);
}

static int array_join(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int len;
	int j = 0;
	MooObject *obj;
	MooObjectArray *m_this;
	char filler[STRING_SIZE];
	char buffer[LARGE_STRING_SIZE];

	if (!(m_this = dynamic_cast<MooObjectArray *>(args->m_this)))
		throw moo_method_object;
	if (args->m_args->last() != 0)
		throw moo_args_mismatched;
	m_this->check_throw(MOO_PERM_R);
	obj = args->m_args->get(0);
	len = obj->to_string(filler, STRING_SIZE);

	if (m_this->last() < 0) {
		args->m_result = new MooString("");
		return(0);
	}

	obj = m_this->get(0);
	j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	for (int i = 1; i <= m_this->last() && j < LARGE_STRING_SIZE; i++) {
		strncpy(&buffer[j], filler, LARGE_STRING_SIZE - j);
		j += len;
		obj = m_this->get(i);
		j += obj->to_string(&buffer[j], LARGE_STRING_SIZE - j);
	}
	buffer[j] = '\0';
	args->m_result = new MooString("%s", buffer);
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
		MooObject *obj;

		if (m_index > m_this->last())
			return(0);

		// TODO this is disabled for now because it causes trouble if you don't pay attention to the return type
		/// Stop this loop if the previous cycle returned #f
		//if ((obj = frame->get_return()) && !obj->is_true())
		//	return(0);

		if (m_index < m_this->last())
			frame->push_event(new ArrayEventForeach(m_index + 1, m_this, m_env, m_func));
		args = new MooArgs();
		args->m_args->set(0, MOO_INCREF(m_func));
		args->m_args->set(1, MOO_INCREF(m_this->get(m_index)));
		frame->push_event(new MooCodeEventCallFunc(m_env, args));
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
	m_this->check_throw(MOO_PERM_R);
	func = args->m_args->get(0);
	args->m_result = new MooBoolean(B_TRUE);
	frame->push_event(new ArrayEventForeach(0, m_this, env, func));
	return(0);
}

void moo_load_array_methods(MooObjectHash *env)
{
	env->set("get", new MooFunc(array_get));
	env->set("set", new MooFunc(array_set));
	env->set("remove", new MooFunc(array_remove));
	env->set("push", new MooFunc(array_push));
	env->set("pop", new MooFunc(array_pop));
	env->set("shift", new MooFunc(array_shift));
	env->set("unshift", new MooFunc(array_unshift));
	env->set("insert", new MooFunc(array_insert));
	env->set("splice", new MooFunc(array_splice));

	env->set("search", new MooFunc(array_search));
	env->set("join", new MooFunc(array_join));

	env->set("foreach", new MooFunc(array_foreach));
}

