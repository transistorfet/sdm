/*
 * Header Name:	frame.h
 * Description:	MooCode Frame
 */

#ifndef _SDM_CODE_FRAME_H
#define _SDM_CODE_FRAME_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>
#include <sdm/code/event.h>

class MooCodeFrame : public MooObject {
	MooArray<MooCodeEvent *> *m_stack;
	MooObject *m_return;
	MooException *m_exception;
	MooObjectHash *m_env;

    public:
	MooCodeFrame(MooObjectHash *parent = NULL);
	virtual ~MooCodeFrame();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int push_event(MooCodeEvent *event);
	int push_block(MooObjectHash *env, MooCodeExpr *expr);
	int push_call(MooObjectHash *env, MooObject *func, MooArgs *args);
	int push_code(const char *code);
	int push_debug(const char *msg, ...);
	int push_return_point();
	int run();
	int handle_exception();
	int rewind_stack(int level = 1);

	int eval(const char *code, MooArgs *args = NULL);

	void set_return(MooObject *obj);
	MooObject *get_return() { return(m_return); }		// TODO should this destroy a reference
	MooObjectHash *env() { return(m_env); }
	void env(MooObjectHash *env);

	int linecol(int &line, int &col);
	void print_stack();
};

extern MooObjectType moo_code_frame_obj_type;

MooObject *make_moo_code_frame(MooDataFile *data);

#endif

