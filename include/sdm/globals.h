/*
 * Header Name:	globals.h
 * Description:	Global Definitions
 */

#ifndef _SDM_GLOBALS_H
#define _SDM_GLOBALS_H

#include <string.h>
#include <sdm/exception.h>

#define STRING_SIZE		512
#define LARGE_STRING_SIZE	2048

#define SDM_FAILED		-1
#define SDM_FATAL		-2

typedef void (*destroy_t)(void *);
typedef int (*compare_t)(void *, void *);
typedef int (*callback_t)(void *, void *);

#define DECLARE_UNUSED(arg)	((void) arg)

#define TRIM_WHITESPACE(str, i) \
	for (; ((str)[(i)] == ' ') || ((str)[(i)] == '\t'); (i)++) ;

typedef int moo_id_t;
typedef int moo_perm_t;

// TODO do remaining
#define MOO_PERM_R		0x01

#define MOO_DEFAULT_PERMS	MOO_PERM_R /* TODO what are the defaults */

#define SBIT(bits, flag)	( (bits) |= (flag) )
#define RBIT(bits, flag)	( (bits) &= ~(flag) )

#endif


