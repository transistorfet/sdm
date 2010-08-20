/*
 * Object Name:	action.c
 * Description:	Action Base Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

MooObjectType moo_action_obj_type = {
	NULL,
	"action",
	typeid(MooAction).name(),
	(moo_type_create_t) NULL
};

MooArgs::MooArgs()
{
	m_action = NULL;
	m_result = NULL;
	m_user = NULL;
	m_caller = NULL;
	m_this = NULL;
	m_object = NULL;
	m_target = NULL;
	m_text = NULL;
}

MooAction::MooAction(const char *name, MooThing *owner)
{
	this->init(name, owner);
}

MooAction::~MooAction()
{
	// TODO maybe you could check the owner and make sure the action is removed???
	if (m_name)
		delete m_name;
}

void MooAction::init(const char *name, MooThing *owner)
{
	m_name = name ? new std::string(name) : NULL;
	m_owner = owner;
}


