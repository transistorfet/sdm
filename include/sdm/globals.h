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

struct property_s {
	char *name;
	char *value;
	struct property_s *next;
};

static inline char *GET_PROPERTY(struct property_s *prop, const char *name) {
	while (prop) {
		if (!strcmp(prop->name, name))
			return(prop->value);
		prop = prop->next;
	}
	return(NULL);
}

#define DECLARE_UNUSED(arg)	((void) arg)

#endif


