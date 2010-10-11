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

//#define BUILTIN_LIST_SIZE	32
//#define BUILTIN_LIST_BITS	MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL | MOO_HBF_DELETE

MooObjectType moo_code_obj_type = {
	&moo_action_obj_type,
	"builtin",
	typeid(MooCode).name(),
	(moo_type_create_t) moo_code_create
};

//static MooCodeHash *builtin_actions = NULL;

int init_moo_code(void)
{
	if (moo_object_register_type(&moo_code_obj_type) < 0)
		return(-1);
/*
	if (builtin_actions)
		return(1);
	builtin_actions = new MooCodeHash(BUILTIN_LIST_SIZE, BUILTIN_LIST_BITS);
	moo_load_basic_actions(builtin_actions);
	moo_load_builder_actions(builtin_actions);
	moo_load_channel_actions(builtin_actions);
	moo_load_item_actions(builtin_actions);
	moo_load_mobile_actions(builtin_actions);
	moo_load_room_actions(builtin_actions);
	moo_load_user_actions(builtin_actions);
*/
	return(0);
}

void release_moo_code(void)
{
/*
	if (!builtin_actions)
		return;
	delete builtin_actions;
	builtin_actions = NULL;
*/
	moo_object_deregister_type(&moo_code_obj_type);
}

MooObject *moo_code_create(void)
{
	return(new MooCode());
}

MooCode::MooCode(MooCodeExpr *code, const char *params, const char *name, MooThing *thing) : MooAction(name, thing)
{
	m_code = code;
	if (params)
		this->params(params);
}

int MooCode::read_entry(const char *type, MooDataFile *data)
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

int MooCode::write_data(MooDataFile *data)
{
	const char *name;

	MooObject::write_data(data);
	// TODO write the code to the file
	//data->write_string_entry("code", name);
	data->write_string_entry("params", this->params());
	return(0);
}

int MooCode::do_action(MooThing *thing, MooArgs *args)
{
/*
	if (!m_func)
		return(-1);
	return(m_func(this, thing, args));
*/
}

int MooCode::set(const char *name)
{
	// TODO wtf
	//m_code = MooCodeParser::parse(name);
	return(0);
}



