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

#define DEFAULT_ARGS	5

class MooUser;
class MooThing;
class MooAction;
class MooChannel;
class MooCodeExpr;

class MooArgs : public MooObject {
    public:
	MooThing *m_user;
	MooThing *m_channel;
	MooObject *m_this;
	MooObject *m_result;
	MooObjectArray *m_args;		// TODO better name!??

	MooArgs(int init_size = DEFAULT_ARGS, MooThing *user = NULL, MooThing *channel = NULL);
	MooArgs(MooObjectArray *args, MooThing *user = NULL, MooThing *channel = NULL);
	virtual ~MooArgs();
	void init(MooThing *user, MooThing *channel, MooObject *thing = NULL);
	void set_args(MooObjectArray *&args);
	int map_args(MooObjectHash *env, MooCodeExpr *params);

	static int find_whitespace(const char *text);
	static int find_character(const char *text);
	static const char *parse_word(char *buffer, int max, const char *text);
	static char *parse_word(char *buffer);

	MooObjectHash *make_env(MooObjectHash *env = NULL);

    private:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
};

extern MooObjectType moo_args_obj_type;

MooObject *moo_args_create(void);

#endif

