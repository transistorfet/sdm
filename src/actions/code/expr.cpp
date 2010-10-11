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
#include "thread.h"

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

int MooCodeExpr::eval(MooCodeThread *thread)
{
	// TODO should this run the expr in the current thread or something!??
	switch (m_type) {
	    case MCT_OBJECT:
		// TODO should you make a reference to m_value or something?  Should set_return?
		thread->set_return(m_value);
		break;
	    case MCT_IDENTIFIER:
		// TODO do variable lookup, use thread to find the evironment/table to look up
		break;
	    case MCT_CALL:
		// TODO m_value should be a string?? Or should it be a list of exprs with the first expr being an identifier?
		// TODO either look up the function yourself or just call thread->call(name, whatever)
		break;
	    case MCT_CODE:
		// TODO same as above to get code name or expr itself or whatever
		// TODO pass remaining expr to code (how will expr be called? will an event be added to the stack?)
		break;
	    default:
		return(-1);
	}
	return(0);
}


