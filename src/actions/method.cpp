/*
 * Name:	method.cpp
 * Description:	MooCode Action
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/code/code.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/actions/method.h>

MooObjectType moo_method_obj_type = {
	NULL,
	"method",
	typeid(MooMethod).name(),
	(moo_type_create_t) moo_method_create
};

MooObject *moo_method_create(void)
{
	return(new MooMethod(NULL, NULL));
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


