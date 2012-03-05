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

#define MOO_FRAME_CYCLE_LIMIT		50000

extern class MooCodeFrame *g_current_frame;

class MooCodeFrame {
	moo_id_t m_owner;
	MooArray<MooCodeEvent *> *m_stack;
	MooCodeEvent *m_curevent;
	MooObject *m_return;
	MooException *m_exception;
	MooObjectHash *m_env;

    public:
	MooCodeFrame(MooObjectHash *env = NULL);
	virtual ~MooCodeFrame();
	int clear();

	int eval(const char *code, MooObjectArray *args = NULL);

	int push_event(MooCodeEvent *event);
	int push_block(MooObjectHash *env, MooCodeExpr *expr);
	int push_call(MooObjectHash *env, MooObjectArray *args);
	int push_call(MooObjectHash *env, MooObject *func, MooObjectArray *args);
	int push_method_call(const char *name, MooObject *obj, MooObject *arg1 = NULL, MooObject *arg2 = NULL, MooObject *arg3 = NULL);
	int push_code(const char *code);
	int push_debug(const char *msg, ...);

	static int run_all();
	int schedule(double time);
	int run();
	int do_run(int limit = 0);
	int mark_return_point();
	int goto_return_point(int level = 1);
	int mark_exception(MooCodeExpr *handler);
	int handle_exception();

	int elevate(MooObject *obj);

	void set_return(MooObject *obj);
	MooObject *get_return() { return(m_return); }		// TODO should this destroy a reference
	MooObjectHash *env() { return(m_env); }
	void env(MooObjectHash *env);
	MooObjectHash *extend_env();
	MooObject *resolve(const char *name, MooObject *value = NULL, MooObject **parent = NULL);

	void print_stacktrace();

    public:
	moo_id_t owner() { return(m_owner); }
	static moo_id_t current_owner() { if (g_current_frame) return(g_current_frame->owner()); return(-1); } 
	static MooCodeFrame *current_frame() { return(g_current_frame); } 

    private:
	friend class FrameEventRelegate;
	// TODO this is just here temporarily until we fix it
	friend int irc_login(MooCodeFrame *frame, class MooTCP *driver, class MooObjectHash *env);
	friend int main(int argc, char **argv);
	moo_id_t owner(moo_id_t owner) { return(m_owner = owner); }
};

class MooCodeFrameSuspend { };

int init_frame(void);
void release_frame(void);

#endif

