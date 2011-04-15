/*
 * Object Name:	lambda.cpp
 * Description:	Moo Code Lambda
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/args.h>
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
	if (!strcmp(type, "func")) {
		char buffer[STRING_SIZE];

		if (data->read_raw_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		m_func = MooCodeParser::parse_code(buffer);
	}
	else if (!strcmp(type, "params")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		m_params = MooCodeParser::parse_code(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooCodeLambda::write_data(MooDataFile *data)
{
	char buffer[STRING_SIZE];

	MooObject::write_data(data);
	MooCodeParser::generate(m_func, buffer, STRING_SIZE);
	// TODO should this write a raw_string instead?
	data->write_begin_entry("code");
	data->write_raw_string(buffer);
	data->write_end_entry();
	MooCodeParser::generate(m_params, buffer, STRING_SIZE);
	data->write_string_entry("params", buffer);
	return(0);
}

int MooCodeLambda::to_string(char *buffer, int max)
{
//	return(snprintf(buffer, max, "%ld", m_num));
	return(0);
}

int MooCodeLambda::do_evaluate(MooCodeFrame *frame, MooObjectHash *parent, MooArgs *args)
{
	MooObjectHash *env;

	env = frame->env();
	env = new MooObjectHash(env);
	args->map_args(env, m_params);
	return(frame->push_block(env, m_func, args));
}


