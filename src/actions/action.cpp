/*
 * Object Name:	action.c
 * Description:	Action Base Object
 */

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/exception.h>

#include <sdm/objs/object.h>
#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/thingref.h>
#include <sdm/things/user.h>
#include <sdm/actions/action.h>

MooObjectType moo_action_obj_type = {
	NULL,
	"action",
	typeid(MooAction).name(),
	(moo_type_create_t) NULL
};

MooAction::MooAction(MooThing *thing)
{
	this->init(thing);
	m_params[0] = '\0';
}

MooAction::~MooAction()
{

}

void MooAction::init(MooThing *thing)
{
	m_thing = thing;
}

const char *MooAction::params(const char *params)
{
	if (params) {
		strncpy(m_params, params, MOO_PARAM_STRING_SIZE);
		m_params[MOO_PARAM_STRING_SIZE - 1] = '\0';
	}
	return(m_params);
}

