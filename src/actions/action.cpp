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
	m_thing = NULL;
	m_object = NULL;
	m_target = NULL;
	m_text = NULL;
}

MooAction::MooAction()
{

}

