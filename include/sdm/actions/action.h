/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooArgs;
class MooUser;
class MooThing;
class MooAction;

class MooAction : public MooObject {
	std::string *m_name;
	MooThing *m_thing;

    public:
	MooAction(const char *name = NULL, MooThing *thing = NULL);
	virtual ~MooAction();
	void init(const char *name = NULL, MooThing *thing = NULL);

	virtual int read_entry(const char *type, MooDataFile *data) = 0;
	virtual int write_data(MooDataFile *data) = 0;

	// TODO should actions run at the permission level of the owner (SUID) or possibly have an SUID bit
	// TODO if we do this, then we will need a function to call which elevates the permissions
	// TODO we also need a way to elevate permissions
	virtual int do_action(MooThing *thing, MooArgs *args) = 0;

    public:
	const char *name() { return(m_name->c_str()); }
	MooThing *thing() { return(m_thing); }
};

class MooArgs {
    public:
	MooAction *m_action;
	const char *m_action_text;
	MooObject *m_result;
	MooThing *m_user;
	MooThing *m_channel;
	MooThing *m_caller;
	MooThing *m_this;
	MooThing *m_object;
	MooThing *m_target;
	const char *m_text;

	MooArgs();
	~MooArgs();

	int set(MooThing *thing, const char *text);

	static int parse_word(char *buffer);
	static int parse_whitespace(char *buffer);
	int parse_words(char *buffer);
	int parse_args(MooThing *user, char *buffer, int max, const char *action, const char *text = NULL);
	int parse_args(MooThing *user, char *buffer, int *argpos = NULL);
	int parse_args(MooThing *user, const char *action, char *buffer);
	int parse_args(MooThing *user, MooThing *object, MooThing *target);
};


extern MooObjectType moo_action_obj_type;

#endif

