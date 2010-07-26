/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

typedef int (*moo_action_t)(class MooAction *, class MooThing *, class MooArgs *);

class MooArgs {
    public:
	const char *action;
	MooObject *result;
	MooThing *thing;
	MooThing *caller;
	MooThing *obj;
	MooThing *target;
	const char *text;
};

class MooAction : public MooObject {
	moo_action_t m_func;
    public:
	MooAction(moo_action_t func = NULL);
	virtual ~MooAction() { }

	virtual int do_action(MooThing *thing, MooArgs *args);

};

extern MooObjectType moo_action_obj_type;

#endif

