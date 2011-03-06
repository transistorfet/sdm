/*
 * Header Name:	code.h
 * Description:	MooCode Action Header
 */

#ifndef _SDM_ACTIONS_CODE_H
#define _SDM_ACTIONS_CODE_H

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/code/code.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

class MooCodeAction : public MooAction {
	MooCodeExpr *m_code;

    public:
	MooCodeAction(MooCodeExpr *code = NULL, const char *params = NULL, const char *name = NULL, MooThing *thing = NULL);
	virtual ~MooCodeAction() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	virtual int do_action(MooArgs *args);

	int set(const char *name);
};

extern MooObjectType moo_code_obj_type;

MooObject *moo_code_create(void);

#endif


