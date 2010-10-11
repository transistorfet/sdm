/*
 * Header Name:	expr.h
 * Description:	MooCode Expression
 */

#ifndef _SDM_ACTIONS_CODE_EXPR_H
#define _SDM_ACTIONS_CODE_EXPR_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

#define MCT_OBJECT	1
#define MCT_IDENTIFIER	2
#define MCT_CALL	3
#define MCT_CODE	4

class MooCodeThread;

class MooCodeExpr : public MooObject {
	int m_type;
	MooObject *m_value;
	MooCodeExpr *m_next;

    public:
	MooCodeExpr(int type, MooObject *value = NULL, MooCodeExpr *next = NULL);
	virtual ~MooCodeExpr();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	int eval(MooCodeThread *thread);

    public:
	MooCodeExpr *next() { return(m_next); }
};

extern MooObjectType moo_code_expr_obj_type;

MooObject *moo_code_expr_create(void);

#endif

