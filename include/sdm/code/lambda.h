/*
 * Object Name:	lambda.h
 * Description:	Moo Code Lambda
 */

#ifndef _SDM_CODE_LAMBDA_H
#define _SDM_CODE_LAMBDA_H

#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/code/expr.h>

class MooCodeLambda : public MooObject {
    public:
	MooCodeExpr *m_func;
	MooCodeExpr *m_params;

	MooCodeLambda(MooCodeExpr *params, MooCodeExpr *func);
	virtual ~MooCodeLambda();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

    protected:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *parent, MooArgs *args);
};

extern MooObjectType moo_code_lambda_obj_type;

MooObject *moo_code_lambda_create(void);

#endif

