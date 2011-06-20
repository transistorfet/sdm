/*
 * Object Name:	args.c
 * Description:	Args Object
 */

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/code/code.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>

MooObjectType moo_args_obj_type = {
	NULL,
	"args",
	typeid(MooArgs).name(),
	(moo_type_make_t) make_moo_args
};

MooObject *make_moo_args(MooDataFile *data)
{
	MooArgs *obj = new MooArgs();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooArgs::MooArgs(int init_size)
{
	m_this = NULL;
	m_result = NULL;
	m_args = new MooObjectArray(init_size);
}

MooArgs::MooArgs(MooObjectArray *args)
{
	m_this = NULL;
	m_result = NULL;
	MOO_INCREF(m_args = args);
}

MooArgs::~MooArgs()
{
	MOO_DECREF(m_result);
	MOO_DECREF(m_args);
}

void MooArgs::set_args(MooObjectArray *&args)
{
	MOO_DECREF(m_args);
	m_args = args;
	args = NULL;
}

int MooArgs::read_entry(const char *type, MooDataFile *data)
{
	moo_status("DATA: Attempting to read unreadable type: MooArgs (%x)", this);
	return(0);
}

int MooArgs::write_data(MooDataFile *data)
{
	moo_status("DATA: Attempting to write unwritable type: MooArgs (%x)", this);
	return(0);
}

MooObject *MooArgs::access_property(const char *name, MooObject *value)
{
	if (!strcmp(name, "args")) {
		if (value) {
			MooObjectArray *obj;
			if (!(obj = dynamic_cast<MooObjectArray *>(value)))
				throw moo_type_error;
			m_args = obj;
		}
		return(m_args);
	}
	else if (!strcmp(name, "this")) {
		if (value)
			m_this = value;
		return(m_this);
	}
	else if (!strcmp(name, "result")) {
		if (value)
			m_result = value;
		return(m_result);
	}
	return(NULL);
}

