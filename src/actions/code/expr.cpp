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
#include <sdm/actions/action.h>
#include <sdm/actions/code/code.h>

#include "expr.h"
#include "frame.h"

MooObjectType moo_code_expr_obj_type = {
	&moo_object_obj_type,
	"expr",
	typeid(MooCodeExpr).name(),
	(moo_type_create_t) moo_code_expr_create
};


MooObject *moo_code_expr_create(void)
{
	return(new MooCodeExpr(0));
}

MooCodeExpr::MooCodeExpr(int type, MooObject *value, MooCodeExpr *next)
{
	m_type = type;
	m_value = value;
	m_next = next;
}

MooCodeExpr::~MooCodeExpr()
{
	if (m_value)
		delete m_value;
	if (m_next)
		delete m_next;
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
	const char *name;

	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	//data->write_string_entry("params", this->params());
	return(0);
}

int MooCodeExpr::evaluate(MooArgs *args)
{
	// TODO we should create a frame and evaluate ourself in it??  This really isn't what a function would normally be
}

const char *MooCodeExpr::lineinfo()
{
	// TODO this should return the action/file/linenumber/columnnumber/etc to be printed for debug purposes
	return("Unknown");
}

