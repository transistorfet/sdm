/*
 * Object Name:	lambda.h
 * Description:	Moo Code Lambda
 */

#ifndef _SDM_ACTION_CODE_LAMBDA_H
#define _SDM_ACTION_CODE_LAMBDA_H

#include <stdarg.h>

#include <sdm/objs/object.h>
#include "expr.h"

class MooCodeLambda : public MooObject {
    public:
	MooCodeExpr *m_func;
	MooCodeExpr *m_params;

	MooCodeLambda(MooCodeExpr *func, MooCodeExpr *params = NULL) { m_func = func; m_params = params; }
	virtual ~MooCodeLambda() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);
	virtual int evaluate(MooObjectHash *parent, MooArgs *args);
};

extern MooObjectType moo_code_lambda_obj_type;

MooObject *moo_code_lambda_create(void);

#endif

