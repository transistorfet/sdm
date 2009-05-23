/*
 * Header Name:	globals.h
 * Description:	Global Definitions
 */

#ifndef _SDM_GLOBALS_H
#define _SDM_GLOBALS_H

#include <string.h>

#define STRING_SIZE		512
#define LARGE_STRING_SIZE	2048

#define SDM_FAILED		-1
#define SDM_FATAL		-2

typedef void (*destroy_t)(void *);
typedef int (*compare_t)(void *, void *);
typedef int (*callback_t)(void *, void *);

struct callback_s {
	callback_t func;
	void *ptr;
};

#define EXECUTE_CALLBACK(callback, arg)	\
	if ((callback).func)			\
		((callback).func)((callback).ptr, arg)

#define DECLARE_UNUSED(arg)	((void) arg)

#define TRIM_WHITESPACE(str, i) \
	for (; ((str)[(i)] == ' ') || ((str)[(i)] == '\t'); (i)++) ;

#endif


