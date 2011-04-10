/*
 * Name:	code.cpp
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
#include <sdm/objs/object.h>
#include <sdm/objs/thingref.h>

#include <sdm/things/thing.h>
#include <sdm/actions/code.h>

MooObjectType moo_code_obj_type = {
	&moo_code_lambda_obj_type,
	"moocode",
	typeid(MooCodeAction).name(),
	(moo_type_create_t) moo_code_create
};

MooObject *moo_code_create(void)
{
	return(new MooCodeAction(NULL, NULL));
}

int MooCodeAction::read_entry(const char *type, MooDataFile *data)
{
	return(MooCodeLambda::read_entry(type, data));
}

int MooCodeAction::write_data(MooDataFile *data)
{
	return(MooCodeLambda::write_data(data));
}

int MooCodeAction::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	// TODO what do you do here??
	return(MooCodeLambda::do_evaluate(frame, env, args));
}


