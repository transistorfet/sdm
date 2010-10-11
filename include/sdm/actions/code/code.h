/*
 * Header Name:	code.h
 * Description:	MooCode Header
 */

#ifndef _SDM_ACTIONS_CODE_CODE_H
#define _SDM_ACTIONS_CODE_CODE_H

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

class MooCodeExpr;

class MooCode : public MooAction {
	MooCodeExpr *m_code;
    public:
	MooCode(MooCodeExpr *code = NULL, const char *params = NULL, const char *name = NULL, MooThing *thing = NULL);
	virtual ~MooCode() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	virtual int do_action(MooThing *thing, MooArgs *args);

	int set(const char *name);
};

//typedef MooHash<MooCode *> MooCodeHash;

extern MooObjectType moo_code_obj_type;

int init_moo_code(void);
void release_moo_code(void);
MooObject *moo_code_create(void);

#endif


