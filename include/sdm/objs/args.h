/*
 * Object Name:	args.h
 * Description:	Args Object
 */

#ifndef _SDM_OBJS_ARGS_H
#define _SDM_OBJS_ARGS_H

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

class MooUser;
class MooThing;
class MooAction;
class MooChannel;

class MooArgs : public MooObject {
    public:
	MooArgs *m_parent;
	MooAction *m_action;
	MooObject *m_result;
	MooThing *m_user;
	MooThing *m_channel;
	MooThing *m_caller;		// TODO unused at this point
	MooThing *m_this;
	MooObjectArray *m_args;		// TODO better name!??

	MooArgs(int init_size = 5, MooThing *user = NULL, MooThing *channel = NULL);
	MooArgs(MooObjectArray *&args, MooThing *user = NULL, MooThing *channel = NULL);
	MooArgs(MooArgs *args, int init_size = 5);
	virtual ~MooArgs();
	void init(MooThing *user, MooThing *channel, MooThing *caller, MooThing *thing, MooArgs *parent, MooAction *action);
	void set_args(MooObjectArray *&args);

	static int find_whitespace(const char *text);
	static int find_character(const char *text);
	static const char *parse_word(char *buffer, int max, const char *text);
	static char *parse_word(char *buffer);

	void init(MooThing *user, MooThing *channel);
	int parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer, int max, const char *text);
	int parse_args(const char *params, MooThing *user, MooThing *channel, char *buffer);
	int match_args(const char *params);
	void match_args_throw(const char *params);
	const MooObjectType *get_type(char param);

	virtual MooObject *access_property(const char *name, MooObject *value = NULL);

	/// Accessors
	inline MooObject *get(int index, MooObjectType *type) { return(m_args->get(index, type)); }
	inline long int get_integer(int index) { return(m_args->get_integer(index)); }
	inline double get_float(int index) { return(m_args->get_float(index)); }
	inline const char *get_string(int index) { return(m_args->get_string(index)); }
	inline MooThing *get_thing(int index) { return(m_args->get_thing(index)); }

	inline int set(int index, MooObject *obj) { if (!m_args->set(index, obj)) return(-1); return(0); }
};

extern MooObjectType moo_args_obj_type;

MooObject *moo_args_create(void);

#endif

