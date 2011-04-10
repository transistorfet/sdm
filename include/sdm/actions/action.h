/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/args.h>


class MooUser;
class MooThing;
class MooAction;
class MooChannel;

class MooAction {
    protected:
	MooThing *m_thing;

    public:
	MooAction(MooThing *thing = NULL) { this->init(thing); }
	virtual ~MooAction() { }
	void init(MooThing *thing = NULL) { m_thing = thing; }

    public:
	MooThing *thing() { return(m_thing); }
};

#endif

