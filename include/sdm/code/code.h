/*
 * Header Name:	code.h
 * Description:	MooCode Header
 */

#ifndef _SDM_CODE_CODE_H
#define _SDM_CODE_CODE_H

#include <sdm/globals.h>

#include <sdm/objs/object.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/array.h>
#include <sdm/objs/nil.h>
#include <sdm/objs/args.h>
#include <sdm/objs/boolean.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

#include <sdm/funcs/func.h>
#include <sdm/funcs/method.h>

#include <sdm/code/expr.h>
#include <sdm/code/event.h>
#include <sdm/code/frame.h>
#include <sdm/code/parser.h>
#include <sdm/code/lambda.h>

extern MooObjectHash *global_env;

int init_moo_code(void);
void release_moo_code(void);

#endif


