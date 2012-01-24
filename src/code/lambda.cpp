/*
 * Object Name:	lambda.cpp
 * Description:	Moo Code Lambda
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/code/code.h>

struct MooObjectType moo_code_lambda_obj_type = {
	"moolambda",
	typeid(MooCodeLambda).name(),
	(moo_type_load_t) load_moo_code_lambda
};

MooObject *load_moo_code_lambda(MooDataFile *data)
{
	MooCodeLambda *obj = new MooCodeLambda(NULL, NULL);
	if (data)
		obj->read_data(data);
	return(obj);
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
	if (!strcmp(type, "code")) {
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
	MooCodeParser::generate(m_params, buffer, STRING_SIZE);
	data->write_begin_entry("params");
	data->write_raw_string(buffer);
	data->write_end_entry();

	buffer[0] = '\n';
	MooCodeParser::generate(m_func, &buffer[1], STRING_SIZE);
	data->write_begin_entry("code");
	data->write_raw_string(buffer);
	data->write_end_entry();
	return(0);
}

int MooCodeLambda::to_string(char *buffer, int max)
{
	int i = 0;

	strcpy(buffer, "(lambda (");
	i += 9;
	i += MooCodeParser::generate(m_params, &buffer[i], STRING_SIZE - i, &moo_style_one_line);
	strcpy(&buffer[i], ") ");
	i += 2;
	i += MooCodeParser::generate(m_func, &buffer[i], STRING_SIZE -i, &moo_style_one_line);
	buffer[i] = ')';
	return(0);
}

int MooCodeLambda::map_args(MooObjectHash *env, MooObjectArray *args)
{
	int i;
	const char *id;
	MooCodeExpr *cur;

	for (i = 0, cur = m_params; cur; i++, cur = cur->next()) {
		id = cur->get_identifier();
		if (!strcmp(id, "&args")) {
			env->set("args", args);
			return(0);
		}
		else if (!strcmp(id, "&rest")) {
			MooObjectArray *array = new MooObjectArray();
			for (; i <= args->last(); i++)
				array->push(MOO_INCREF(args->get(i)));
			env->set("rest", array);
			return(0);
		}
		else {
			if (i > args->last())
				break;
			env->set(id, args->get(i));
		}
	}
	if (cur || i <= args->last())
		throw moo_args_mismatched;
	return(0);
}


int MooCodeLambda::do_evaluate(MooCodeFrame *frame, MooObjectHash *parent, MooObjectArray *args)
{
	MooObjectHash *env;

	env = frame->env();
	env = new MooObjectHash(env);
	this->map_args(env, args);
	frame->mark_return_point();
	return(frame->push_block(env, m_func));
}


