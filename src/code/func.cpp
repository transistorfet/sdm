/*
 * Object Name:	func.cpp
 * Description:	Moo Code Function
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/objs/object.h>

struct MooObjectType moo_code_func_obj_type = {
	NULL,
	"moofunc",
	typeid(MooCodeFunc).name(),
	(moo_type_create_t) moo_code_func_create
};

MooObject *moo_code_func_create(void)
{
	return(new MooCodeFunc((moo_code_func_t) NULL));
}

int MooCodeFunc::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "func")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		// TODO how the fuck does this all work?
		MooCodeFunc *func = dynamic_cast<MooCodeFunc *>(global_env->get(buffer));
		if (!func)
			throw MooException("MooCodeFunc not found, %s", buffer);
		m_func = func->m_func;
		//m_string = ???
	}
	else if (!strcmp(type, "params")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		m_params = MooCodeParser::parse_code(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooCodeFunc::write_data(MooDataFile *data)
{
	char buffer[STRING_SIZE];

	MooObject::write_data(data);
	// TODO how do you find the function name??
	//data->write_string_entry("func", buffer);
	MooCodeParser::generate(m_params, buffer, STRING_SIZE);
	data->write_string_entry("params", buffer);
	return(0);
}

int MooCodeFunc::to_string(char *buffer, int max)
{
//	return(snprintf(buffer, max, "%ld", m_num));
	return(0);
}

int MooCodeFunc::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (!m_func)
		throw MooException("Null function");
	return(m_func(frame, env, args));
}

