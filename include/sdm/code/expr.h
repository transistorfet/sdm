/*
 * Header Name:	expr.h
 * Description:	MooCode Expression
 */

#ifndef _SDM_CODE_EXPR_H
#define _SDM_CODE_EXPR_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define MCT_OBJECT	1
#define MCT_IDENTIFIER	2
#define MCT_CALL	3

class MooCodeFrame;

typedef union {
	int linecol;
	struct {
		short line;
		short col;
	};
} linecol_t;

class MooCodeExpr : public MooObject {
    protected:
	linecol_t m_linecol;
	int m_type;
	MooObject *m_value;
	MooCodeExpr *m_next;
	// TODO we could store a string here which has all the whitespace between what was parsed into this expr and the next token
	//	which we can use when regenerating the code to get the exact same formatting

    public:
	MooCodeExpr(int line, int col, int type, MooObject *value = NULL, MooCodeExpr *next = NULL);
	virtual ~MooCodeExpr();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectArray *args);

    public:
	int line() { return(m_linecol.line); }
	int col() { return(m_linecol.col); }
	MooCodeExpr *next() { return(m_next); }
	int type() { return(m_type); }
	MooObject *value() { return(m_value); }

	const char *get_identifier();
	MooCodeExpr *get_call();

	const char *get_string();

	static int check_args(MooCodeExpr *expr, int min, int max);

    protected:
	friend class MooCodeParser;
	void next(MooCodeExpr *expr) { if (expr) m_next = expr; }
};

extern MooObjectType moo_code_expr_obj_type;

MooObject *load_moo_code_expr(MooDataFile *data);

#endif

