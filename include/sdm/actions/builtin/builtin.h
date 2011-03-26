/*
 * Header Name:	builtin.h
 * Description:	Builtin Stuff Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BUILTIN_H
#define _SDM_ACTIONS_BUILTIN_BUILTIN_H

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

typedef int (*moo_action_t)(class MooAction *, class MooThing *, class MooArgs *);

class MooBuiltin : public MooAction {
	moo_action_t m_func;
	MooBuiltin *m_master;
    public:
	MooBuiltin(moo_action_t func = NULL, const char *params = NULL, const char *name = NULL, MooThing *thing = NULL);
	virtual ~MooBuiltin() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	virtual int evaluate(MooObjectHash *env, MooArgs *args);

	int set(const char *name);
};

typedef MooHash<MooBuiltin *> MooBuiltinHash;

extern MooObjectType moo_builtin_obj_type;

int init_builtin(void);
void release_builtin(void);
MooObject *moo_builtin_create(void);

#endif

