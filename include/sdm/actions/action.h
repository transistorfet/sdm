/*
 * Object Name:	action.h
 * Description:	Action Base Object
 */

#ifndef _SDM_ACTIONS_ACTION_H
#define _SDM_ACTIONS_ACTION_H

#include <stdarg.h>

#include <sdm/array.h>
#include <sdm/objs/object.h>

/**
TODO What kind of argument patterns are there:
	- single string		(first arg must be string)
	- list of strings	(NOTE: a list of strings is directly equivallent to a list of MooObjects) (all args must be string or nil)
	- list of MooObjects	(any combination...)
		- you could have a type for each element: number, string, thingref, list, any
	- natural language processing...?? (smart object name detection...)

	- not all verbs could be called from the command line...

	s	single string
	s*	string broken into words
	?	anything
	[?*]	list of anything



	perhaps an array of MooObject, and an array of parameter types/etc.  Depending on the defined parameter list, the args can be
	parsed differently (but only when a command line is being interpreted).  If the params = (string), then pass the string.  If
	more, if array of strings, break into substrings.  If anything else, try to selectively match arguments.  The action can then
	also know for certain the composition of the array, and can directly index into it.  

*/

#define MOO_PARAM_STRING_SIZE		16

class MooArgs;
class MooUser;
class MooThing;
class MooAction;
class MooChannel;

class MooAction : public MooObject {
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
	virtual int do_action(MooThing *thing, MooArgs *args) = 0;

    public:
	const char *name() { return(m_name->c_str()); }
	MooThing *thing() { return(m_thing); }
	const char *params(const char *params = NULL);
};

class MooArgs {
    public:
	MooAction *m_action;
	const char *m_action_text;
	MooObject *m_result;
	MooThing *m_user;
	MooThing *m_channel;
	MooThing *m_caller;		// TODO unused at this point
	MooThing *m_this;
	MooObjectArray *m_args;		// TODO better name!??

	MooArgs();
	~MooArgs();

	static int find_whitespace(const char *text);
	static int find_character(const char *text);
	static const char *parse_action(char *buffer, int max, const char *text);
	static char *parse_action(char *buffer);

	int parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer, int max, const char *text);
	int parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer);
	int match_args(const char *params);
	void match_args_throw(const char *params);
	const MooObjectType *get_type(char param);
};

extern MooObjectType moo_action_obj_type;

#endif

