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

// TODO This should be completely changed to be more intergrated and OOPy, but for now, this makes the conversion more straightforward
class MooCallback {
    protected:
	callback_t func;
	void *ptr;
    public:
	MooCallback(callback_t func, void *ptr) { this->func = func; this->ptr = ptr; }

	int call(void *arg) { if (this->func) return(this->func(this->ptr, arg)); }
};

#define DECLARE_UNUSED(arg)	((void) arg)

#define TRIM_WHITESPACE(str, i) \
	for (; ((str)[(i)] == ' ') || ((str)[(i)] == '\t'); (i)++) ;

#endif


