/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooUser;
class MooThing;
class MooAction;

class MooArgs {
    public:
	MooAction *m_action;
	MooObject *m_result;
	MooUser *m_user;
	MooThing *m_caller;
	MooThing *m_this;
	MooThing *m_object;
	MooThing *m_target;
	const char *m_text;

	MooArgs();

	int set(MooThing *thing, const char *text);
};

class MooAction : public MooObject {
	std::string *m_name;
	MooThing *m_owner;
    public:
	MooAction(const char *name = NULL, MooThing *owner = NULL);
	virtual ~MooAction();
	void init(const char *name = NULL, MooThing *owner = NULL);

	virtual int read_entry(const char *type, MooDataFile *data) = 0;
	virtual int write_data(MooDataFile *data) = 0;

	virtual int do_action(MooThing *thing, MooArgs *args) = 0;

    public:
	const char *name() { return(m_name->c_str()); }
	MooThing *owner() { return(m_owner); }
};

extern MooObjectType moo_action_obj_type;

#endif

