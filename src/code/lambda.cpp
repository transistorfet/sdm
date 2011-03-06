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
	return(new MooCodeLambda(NULL));
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

int MooCodeLambda::evaluate(MooObjectHash *parent, MooArgs *args)
{
	int ret;
	MooObjectHash *env;
	MooCodeFrame frame;

	env = frame.env();
	// TODO bind args to params list
	//env->set("args", args);
	//env->set("parent", new MooThingRef(m_thing));
	frame.add_block(args, m_func);
	ret = frame.run();
	args->m_result = frame.get_return();
	return(ret);
}

