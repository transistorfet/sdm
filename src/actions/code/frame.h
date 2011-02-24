/*
 * Header Name:	frame.h
 * Description:	MooCode Frame
 */

#ifndef _SDM_ACTIONS_CODE_FRAME_H
#define _SDM_ACTIONS_CODE_FRAME_H

#include <sdm/array.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

#include "expr.h"
#include "event.h"

class MooCodeFrame : public MooObject {
	MooArray<MooCodeEvent *> *m_stack;
	MooObject *m_return;

	// TODO you should probably have MooArgs here, right?  It could always be NULL if there are no args.
	MooObjectHash *m_env;

    public:
	MooCodeFrame(MooObjectHash *parent = NULL);
	virtual ~MooCodeFrame();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int push_event(MooCodeEvent *event);
	int add_block(MooCodeExpr *expr);
	int add_expr_list(MooCodeExpr *expr);
	int add_call(MooObject *func, MooArgs *args);		// TODO should this instead be MooObjectArray?
	int add_code(MooObject *func, MooCodeExpr *expr);

	int call(const char *name, MooCodeExpr *expr);
	int call(const char *name, MooArgs *args);
	int eval(MooCodeExpr *expr);

	int run(int level = 0);

	void set_return(MooObject *obj);
	MooObject *get_return() { return(m_return); }		// TODO should this destroy a reference
	MooObjectHash *env() { return(m_env); }
	void env(MooObjectHash *env);
};

extern MooObjectType moo_code_frame_obj_type;

MooObject *moo_code_frame_create(void);

#endif

