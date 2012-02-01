/*
 * Object Name:	funcptr.cpp
 * Description:	Moo Function Pointer
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/objs/object.h>
#include <sdm/objs/funcptr.h>

struct MooObjectType moo_func_ptr_obj_type = {
	"funcptr",
	typeid(MooFuncPtr).name(),
	(moo_type_load_t) load_moo_func_ptr
};

MooObject *load_moo_func_ptr(MooDataFile *data)
{
	MooFuncPtr *obj = new MooFuncPtr((moo_func_t) NULL);
	if (data)
		obj->read_data(data);
	return(obj);
}

MooFuncPtr::MooFuncPtr(moo_func_t func)
{
	m_name = NULL;
	m_func = func;
}

MooFuncPtr::~MooFuncPtr()
{
	if (m_name)
		delete m_name;
}

int MooFuncPtr::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "funcptr")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		MooFuncPtr *func = dynamic_cast<MooFuncPtr *>(global_env->get(buffer));
		if (!func)
			throw MooException("MooFuncPtr not found, %s", buffer);
		m_func = func->m_func;
		m_name = new std::string(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooFuncPtr::write_data(MooDataFile *data)
{

	MooObject::write_data(data);
	data->write_string_entry("funcptr", m_name ? m_name->c_str() : "***ERROR***");
	return(0);
}

int MooFuncPtr::to_string(char *buffer, int max)
{
	if (m_name)
		strncpy(buffer, m_name->c_str(), max);
	else
		snprintf(buffer, max, "FUNC:%x", (unsigned int) m_func);
	return(0);
}

int MooFuncPtr::do_evaluate(MooCodeFrame *frame, MooObjectArray *args)
{
	if (!m_func)
		throw MooException("FUNC: Attempted to evaluate an undefined MooFuncPtr (this means I really messed up).");
	return(m_func(frame, args));
}

