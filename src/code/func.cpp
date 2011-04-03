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
/*
	if (!strcmp(type, "value")) {
		m_num = data->read_integer_entry();
	}
	else
		return(MooObject::read_entry(type, data));
*/
	return(MOO_HANDLED);
}

int MooCodeFunc::write_data(MooDataFile *data)
{
/*
	MooObject::write_data(data);
	data->write_integer_entry("value", m_num);
*/
	return(0);
}

int MooCodeFunc::to_string(char *buffer, int max)
{
//	return(snprintf(buffer, max, "%ld", m_num));
	return(0);
}

int MooCodeFunc::do_evaluate(MooObjectHash *env, MooArgs *args)
{
	if (!m_func)
		throw MooException("Null function");
	return(m_func(env, args));
}

