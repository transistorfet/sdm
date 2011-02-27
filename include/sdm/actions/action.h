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

#define MOO_PARAM_STRING_SIZE		16

class MooUser;
class MooThing;
class MooAction;
class MooChannel;

class MooAction : public MooObject {
    protected:
	std::string *m_name;
	MooThing *m_thing;
	char m_params[MOO_PARAM_STRING_SIZE];

    public:
	MooAction(const char *name = NULL, MooThing *thing = NULL);
	virtual ~MooAction();
	void init(const char *name = NULL, MooThing *thing = NULL);

	virtual int read_entry(const char *type, MooDataFile *data) = 0;
	virtual int write_data(MooDataFile *data) = 0;

	// TODO should actions run at the permission level of the owner (SUID) or possibly have an SUID bit
	// TODO if we do this, then we will need a function to call which elevates the permissions
	// TODO we also need a way to elevate permissions
	virtual int do_action(MooArgs *args) = 0;

    public:
	const char *name() { return(m_name->c_str()); }
	MooThing *thing() { return(m_thing); }
	const char *params(const char *params = NULL);
};

extern MooObjectType moo_action_obj_type;

#endif

