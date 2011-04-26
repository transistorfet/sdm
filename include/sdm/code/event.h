/*
 * Header Name:	event.h
 * Description:	MooCode Event
 */

#ifndef _SDM_CODE_EVENT_H
#define _SDM_CODE_EVENT_H

#include <sdm/globals.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>
#include <sdm/code/parser.h>

class MooCodeEvent;
class MooCodeFrame;

int init_code_event(void);
void release_code_event(void);

class MooCodeEvent {
    protected:
	MooObjectHash *m_env;
	MooCodeExpr *m_expr;
	MooArgs *m_args;

    public:
	MooCodeEvent(MooObjectHash *env, MooArgs *args = NULL, MooCodeExpr *expr = NULL);
	virtual ~MooCodeEvent();
	MooObjectHash *env() { return(m_env); }

	virtual int do_event(MooCodeFrame *frame);

	inline int linecol(int &line, int &col) {
		if (!m_expr) {
			line = 0;
			col = 0;
			return(0);
		}
		line = m_expr->line();
		col = m_expr->col();
		return(1);
	}

	virtual void print_debug() {
		int line, col;
		char buffer[LARGE_STRING_SIZE];

		this->linecol(line, col);
		if (m_expr) {
			MooCodeParser::generate(m_expr, buffer, LARGE_STRING_SIZE, ' ');
			moo_status("DEBUG: (%d, %d) %s: %s", line, col, typeid(*this).name(), buffer);
		}
		else
			moo_status("DEBUG: (%d, %d) %s", line, col, typeid(*this).name());
	}
};

class MooCodeEventEvalExpr : public MooCodeEvent {
    public:
	MooCodeEventEvalExpr(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalBlock : public MooCodeEvent {
    public:
	MooCodeEventEvalBlock(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventCallExpr : public MooCodeEvent {
    public:
	MooCodeEventCallExpr(MooObjectHash *env, MooArgs *args) : MooCodeEvent(env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventEvalArgs : public MooCodeEvent {
    public:
	MooCodeEventEvalArgs(MooObjectHash *env, MooArgs *args, MooCodeExpr *expr) : MooCodeEvent(env, args, expr) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventAppendReturn : public MooCodeEvent {
    public:
	MooCodeEventAppendReturn(MooObjectHash *env, MooArgs *args) : MooCodeEvent(env, args, NULL) { };
	int do_event(MooCodeFrame *frame);
};

class MooCodeEventCatch : public MooCodeEvent {
    public:
	MooCodeEventCatch(MooObjectHash *env, MooCodeExpr *expr) : MooCodeEvent(env, NULL, expr) { };

	/// If we try to evaluate this during the normal run loop then just do nothing
	int do_event(MooCodeFrame *frame) { return(0); }
	int handle(MooCodeFrame *frame);
};

class MooCodeEventDebug : public MooCodeEvent {
	std::string m_msg;
    public:
	MooCodeEventDebug(const char *msg) : MooCodeEvent(NULL, NULL, NULL) {
		m_msg = std::string(msg);
	}

	int do_event(MooCodeFrame *frame) { return(0); }

	void print_debug() {
		moo_status("DEBUG: %s", m_msg.c_str());
	}
};

#endif

