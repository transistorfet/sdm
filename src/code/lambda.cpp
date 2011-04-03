/*
 * Object Name:	lambda.cpp
 * Description:	Moo Code Lambda
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/code/code.h>

struct MooObjectType moo_code_lambda_obj_type = {
	NULL,
	"moolambda",
	typeid(MooCodeLambda).name(),
	(moo_type_create_t) moo_code_lambda_create
};

MooObject *moo_code_lambda_create(void)
{
	return(new MooCodeLambda(NULL, NULL));
}

MooCodeLambda::MooCodeLambda(MooCodeExpr *params, MooCodeExpr *func)
{
	MOO_INCREF(m_params = params);
	MOO_INCREF(m_func = func);
}

MooCodeLambda::~MooCodeLambda()
{
	MOO_DECREF(m_func);
	MOO_DECREF(m_params);
}

int MooCodeLambda::read_entry(const char *type, MooDataFile *data)
{
/*
	if (!strcmp(type, "value")) {
		m_num = data->read_integer_entry();
	}
	else
		return(MooObject::read_entry(type, data));
*/
	return(MOO_HANDLED);
}

int MooCodeLambda::write_data(MooDataFile *data)
{
/*
	MooObject::write_data(data);
	data->write_integer_entry("value", m_num);
*/
	return(0);
}

int MooCodeLambda::to_string(char *buffer, int max)
{
//	return(snprintf(buffer, max, "%ld", m_num));
	return(0);
}

int MooCodeLambda::do_evaluate(MooObjectHash *parent, MooArgs *args)
{
	int i;
	MooCodeExpr *cur;
	MooObjectHash *env;
	MooCodeFrame frame;

	env = frame.env();
	for (i = 0, cur = m_params; cur && i < args->m_args->size(); i++, cur = cur->next())
		env->set(cur->get_identifier(), args->m_args->get(i, NULL));
	if (cur || i <= args->m_args->last())
		throw MooException("Mismatched arguments");
	return(frame.call(m_func, args));
}

