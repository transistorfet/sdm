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

class MooCodeAction : public MooCodeLambda, public MooAction {
    public:
	MooCodeAction(MooCodeExpr *params, MooCodeExpr *func, MooThing *thing = NULL) : MooCodeLambda(params, func), MooAction (thing) { }
	virtual ~MooCodeAction() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_code_obj_type;

MooObject *moo_code_create(void);

#endif


