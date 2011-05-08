/*
 * Name:	code.cpp
 * Description:	MooCode
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/code/code.h>

MooObjectType moo_code_expr_obj_type = {
	&moo_object_obj_type,
	"expr",
	typeid(MooCodeExpr).name(),
	(moo_type_make_t) make_moo_code_expr
};


MooObject *make_moo_code_expr(MooDataFile *data)
{
	MooCodeExpr *obj = new MooCodeExpr(-1, -1, 0);
	if (data)
		obj->read_data(data);
	return(obj);
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
	if (!strcmp(type, "code")) {
		MooCodeExpr *expr;
		char buffer[STRING_SIZE];

		if (data->read_raw_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		expr = MooCodeParser::parse_code(buffer);
		// TODO this is a horrible hack...
		m_line = expr->m_line;
		m_col = expr->m_col;
		m_value = expr->m_value;
		m_type = expr->m_type;
		m_next = expr->m_next;
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooCodeExpr::write_data(MooDataFile *data)
{
	char buffer[STRING_SIZE];

	MooObject::write_data(data);

	buffer[0] = '\n';
	MooCodeParser::generate(this, &buffer[1], STRING_SIZE);
	data->write_begin_entry("code");
	data->write_raw_string(buffer);
	data->write_end_entry();
	return(0);
}

int MooCodeExpr::do_evaluate(MooCodeFrame *frame, MooObjectHash *parent, MooArgs *args)
{
	return(frame->push_block(parent, this));
}

const char *MooCodeExpr::get_identifier()
{
	MooString *str;

	if (m_type != MCT_IDENTIFIER || !(str = dynamic_cast<MooString *>(m_value)))
		throw MooException("(%d, %d) Expected identifier", m_line, m_col);
	return(str->m_str);
}

MooCodeExpr *MooCodeExpr::get_call()
{
	MooCodeExpr *expr = NULL;

	if (m_type != MCT_CALL || (m_value && !(expr = dynamic_cast<MooCodeExpr *>(m_value))))
		throw MooException("(%d, %d) Expected expression", m_line, m_col);
	return(expr);
}

int MooCodeExpr::check_args(MooCodeExpr *expr, int min, int max)
{
	int num = 0;

	for (; expr; num++, expr = expr->next())
		;
	if (num >= min && num <= max)
		return(1);
	return(0);
}

