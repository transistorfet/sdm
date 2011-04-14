/*
 * Header Name:	builtin.h
 * Description:	Builtin Stuff Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BUILTIN_H
#define _SDM_ACTIONS_BUILTIN_BUILTIN_H

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>

#define MOO_PARAM_STRING_SIZE		16

typedef int (*moo_action_t)(MooCodeFrame *frame, MooObjectHash *, class MooArgs *);

class MooBuiltin : public MooObject {
	moo_action_t m_func;
	MooBuiltin *m_master;
	char m_params[MOO_PARAM_STRING_SIZE];

    public:
	MooBuiltin(moo_action_t func = NULL, const char *params = NULL);
	virtual ~MooBuiltin() { }
	int set(const char *name);
	const char *params(const char *params = NULL);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_builtin_obj_type;

int init_builtin(void);
void release_builtin(void);
MooObject *moo_builtin_create(void);

#endif

