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

class MooCodeExpr : public MooObject {
    protected:
	int m_type;
	MooObject *m_value;
	MooCodeExpr *m_next;

    public:
	MooCodeExpr(int type, MooObject *value = NULL, MooCodeExpr *next = NULL);
	virtual ~MooCodeExpr();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int evaluate(MooObjectHash *env, MooArgs *args);

    public:
	MooCodeExpr *next() { return(m_next); }
	int expr_type() { return(m_type); }
	MooObject *value() { return(m_value); }
	const char *lineinfo();

	const char *get_identifier();
	MooCodeExpr *get_call();

	void next(MooCodeExpr *expr) { if (expr) m_next = expr; }
};

extern MooObjectType moo_code_expr_obj_type;

MooObject *moo_code_expr_create(void);

#endif

