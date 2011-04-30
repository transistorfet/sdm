/*
 * Name:	method.cpp
 * Description:	Methodifying Object
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/code/code.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/funcs/method.h>

MooObjectType moo_method_obj_type = {
	NULL,
	"method",
	typeid(MooMethod).name(),
	(moo_type_make_t) make_moo_method
};

MooObject *make_moo_method(MooDataFile *data)
{
	MooMethod *obj = new MooMethod(NULL, NULL);
	if (data)
		obj->read_data(data);
	return(obj);
}

MooMethod::MooMethod(MooObject *obj, MooObject *func)
{
	m_obj = obj;
	m_func = func;
}

int MooMethod::read_entry(const char *type, MooDataFile *data)
{
	//return(MooCodeLambda::read_entry(type, data));
}

int MooMethod::write_data(MooDataFile *data)
{
	//return(MooCodeLambda::write_data(data));
}

int MooMethod::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	args->m_this = m_obj;
	frame->push_call(env, m_func, args);
	return(0);
}


