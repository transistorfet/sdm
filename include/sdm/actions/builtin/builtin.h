/*
 * Header Name:	builtin.h
 * Description:	Builtin Stuff Header
 */

#ifndef _SDM_ACTIONS_BUILTIN_BUILTIN_H
#define _SDM_ACTIONS_BUILTIN_BUILTIN_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

class MooBuiltin : public MooAction {

    public:
	MooBuiltin();
	virtual ~MooBuiltin();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	virtual int do_action(MooThing *thing, MooArgs *args);
};

extern MooObjectType moo_builtin_obj_type;

int init_builtin(void);
void release_builtin(void);
MooObject *moo_builtin_create(void);

#endif

