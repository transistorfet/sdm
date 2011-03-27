/*
 * Name:	code.cpp
 * Description:	MooCode
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/things/thing.h>
#include <sdm/code/code.h>

MooObjectType moo_code_expr_obj_type = {
	&moo_object_obj_type,
	"expr",
	typeid(MooCodeExpr).name(),
	(moo_type_create_t) moo_code_expr_create
};


MooObject *moo_code_expr_create(void)
{
	return(new MooCodeExpr(-1, -1, 0));
}

MooCodeExpr::MooCodeExpr(int line, int col, int type, MooObject *value, MooCodeExpr *next)
{
	m_line = line;
	m_col = col;
	m_type = type;
	m_value = value;
	m_next = next;
}

MooCodeExpr::~MooCodeExpr()
{
	MOO_DECREF(m_value);
	MOO_DECREF(m_next);
}

int MooCodeExpr::read_entry(const char *type, MooDataFile *data)
{
/*
	if (!strcmp(type, "code")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->set(buffer);
	}
	else if (!strcmp(type, "params")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->params(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
*/
	return(MOO_HANDLED);
}

int MooCodeExpr::write_data(MooDataFile *data)
{
	//const char *name;

	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	//data->write_string_entry("params", this->params());
	return(0);
}

int MooCodeExpr::evaluate(MooObjectHash *parent, MooArgs *args)
{
	MooCodeFrame frame;
	return(frame.call(this, args));
}

const char *MooCodeExpr::get_identifier()
{
	const char *str;

	if (m_type != MCT_IDENTIFIER || !(str = m_value->get_string()))
		throw MooException("(%s, %s) Expected identifier", m_line, m_col);
	return(str);
}

MooCodeExpr *MooCodeExpr::get_call()
{
	MooCodeExpr *expr;

	if (m_type != MCT_CALL || !(expr = dynamic_cast<MooCodeExpr *>(m_value)))
		throw MooException("(%s, %s) Expected expression", m_line, m_col);
	return(expr);
}

