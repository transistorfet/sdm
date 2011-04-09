/*
 * Name:	code.cpp
 * Description:	MooCode Action
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/code/code.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/actions/action.h>
#include <sdm/actions/code.h>

MooObjectType moo_code_obj_type = {
	&moo_action_obj_type,
	"moocode",
	typeid(MooCodeAction).name(),
	(moo_type_create_t) moo_code_create
};

MooObject *moo_code_create(void)
{
	return(new MooCodeAction());
}

MooCodeAction::MooCodeAction(MooCodeExpr *code, const char *params, MooThing *thing) : MooAction(thing)
{
	m_code = code;
	if (params)
		this->params(params);
}

int MooCodeAction::read_entry(const char *type, MooDataFile *data)
{
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
	return(MOO_HANDLED);
}

int MooCodeAction::write_data(MooDataFile *data)
{
	char buffer[STRING_SIZE];

	MooObject::write_data(data);
	MooCodeParser::generate(m_code, buffer, STRING_SIZE);
	// TODO should this write a raw_string instead?
	data->write_string_entry("code", buffer);
	data->write_string_entry("params", this->params());
	return(0);
}

// TODO is this class even needed anymore?
int MooCodeAction::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	return(m_code->evaluate(frame, env, args));
}

int MooCodeAction::set(const char *code)
{
	MooCodeParser parser;

	try {
		m_code = parser.parse(code);
		// TODO temporary
		MooCodeParser::print(m_code);
	}
	catch (MooException e) {
		moo_status("%s", e.get());
		return(-1);
	}
	return(0);
}


