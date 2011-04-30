/*
 * Object Name:	func.cpp
 * Description:	Moo Code Function
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/objs/object.h>
#include <sdm/objs/args.h>
#include <sdm/funcs/func.h>

struct MooObjectType moo_func_obj_type = {
	NULL,
	"moofunc",
	typeid(MooFunc).name(),
	(moo_type_make_t) make_moo_func
};

MooObject *make_moo_func(MooDataFile *data)
{
	MooFunc *obj = new MooFunc((moo_func_t) NULL);
	if (data)
		obj->read_data(data);
	return(obj);
}

MooFunc::MooFunc(moo_func_t func)
{
	m_name = NULL;
	m_func = func;
}

MooFunc::~MooFunc()
{
	if (m_name)
		delete m_name;
}

int MooFunc::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "func")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		MooFunc *func = dynamic_cast<MooFunc *>(global_env->get(buffer));
		if (!func)
			throw MooException("MooFunc not found, %s", buffer);
		m_func = func->m_func;
		m_name = new std::string(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooFunc::write_data(MooDataFile *data)
{

	MooObject::write_data(data);
	data->write_string_entry("func", m_name ? m_name->c_str() : "***ERROR***");
	return(0);
}

int MooFunc::to_string(char *buffer, int max)
{
//	return(snprintf(buffer, max, "%ld", m_num));
	return(0);
}

int MooFunc::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (!m_func)
		throw MooException("Null function");
	return(m_func(frame, env, args));
}

