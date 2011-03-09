/*
 * Header Name:	code.h
 * Description:	MooCode Header
 */

#ifndef _SDM_CODE_CODE_H
#define _SDM_CODE_CODE_H

#include <sdm/hash.h>
#include <sdm/globals.h>

#include <sdm/code/func.h>
#include <sdm/code/expr.h>
#include <sdm/code/event.h>
#include <sdm/code/frame.h>
#include <sdm/code/parser.h>
#include <sdm/code/lambda.h>

extern MooObjectHash *global_env;

int init_moo_code(void);
void release_moo_code(void);

#endif


