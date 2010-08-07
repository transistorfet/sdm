/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooThing;
class MooUser;

class MooArgs {
    public:
	const char *m_action;
	MooObject *m_result;
	MooUser *m_user;
	MooThing *m_caller;
	MooThing *m_thing;
	MooThing *m_object;
	MooThing *m_target;
	const char *m_text;

	MooArgs();

	int set(MooThing *thing, const char *text);
};

class MooAction : public MooObject {
    public:
	MooAction();
	virtual ~MooAction() { }

	virtual int do_action(MooThing *thing, MooArgs *args) = 0;
};

extern MooObjectType moo_action_obj_type;

#endif

