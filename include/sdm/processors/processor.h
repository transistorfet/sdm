/*
 * Object Name:	processor.h
 * Description:	Input Processor Object
 */

#ifndef _SDM_PROCESSORS_PROCESSOR_H
#define _SDM_PROCESSORS_PROCESSOR_H

#include <stdarg.h>

#include <sdm/objs/object.h>

#define SDM_PROCESSOR(ptr)		( (struct sdm_processor *) (ptr) )
#define SDM_PROCESSOR_TYPE(ptr)		( (struct sdm_processor_type *) (ptr) )

struct sdm_user;
struct sdm_processor;

typedef int (*sdm_processor_startup_t)(struct sdm_processor *, struct sdm_user *);
typedef int (*sdm_processor_process_t)(struct sdm_processor *, struct sdm_user *, char *);
typedef int (*sdm_processor_shutdown_t)(struct sdm_processor *, struct sdm_user *);

struct sdm_processor {
	struct sdm_object object;

};

struct sdm_processor_type {
	struct sdm_object_type object_type;
	sdm_processor_startup_t startup;
	sdm_processor_process_t process;
	sdm_processor_shutdown_t shutdown;
};

extern struct sdm_processor_type sdm_processor_obj_type;

int sdm_processor_init(struct sdm_processor *, int, va_list);
void sdm_processor_release(struct sdm_processor *);

static inline int sdm_processor_startup(struct sdm_processor *proc, struct sdm_user *user) {
	if (SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->startup)
		return(SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->startup(proc, user));
	return(0);
}

static inline int sdm_processor_process(struct sdm_processor *proc, struct sdm_user *user, char *input) {
	if (SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->process)
		return(SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->process(proc, user, input));
	return(0);
}

static inline int sdm_processor_shutdown(struct sdm_processor *proc, struct sdm_user *user) {
	if (SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->shutdown)
		return(SDM_PROCESSOR_TYPE(SDM_OBJECT(proc)->type)->shutdown(proc, user));
	return(0);
}

#endif

