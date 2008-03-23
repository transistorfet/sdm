/*
 * Object Name:	interpreter.h
 * Description:	Command Interpreter Object
 */

#ifndef _SDM_OBJS_INTERPRETER_H
#define _SDM_OBJS_INTERPRETER_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>

#define SDM_CMD_CLOSE		1337

#define SDM_INTERPRETER(ptr)		( (struct sdm_interpreter *) (ptr) )

struct sdm_user;
typedef int (*sdm_command_t)(void *, struct sdm_user *, char *);

struct sdm_interpreter {
	struct sdm_processor processor;
	struct sdm_hash *commands;

};

extern struct sdm_processor_type sdm_interpreter_obj_type;

int init_interpreter(void);
void release_interpreter(void);

int sdm_interpreter_init(struct sdm_interpreter *, va_list);
void sdm_interpreter_release(struct sdm_interpreter *);

int sdm_interpreter_startup(struct sdm_interpreter *, struct sdm_user *);
int sdm_interpreter_process(struct sdm_interpreter *, struct sdm_user *, char *);
int sdm_interpreter_shutdown(struct sdm_interpreter *, struct sdm_user *);

int sdm_interpreter_add(struct sdm_interpreter *, const char *, sdm_command_t, void *, destroy_t);

int sdm_interpreter_get_string(struct sdm_thing *, const char *, int *);
struct sdm_thing *sdm_interpreter_get_object(struct sdm_thing *, const char *, int *);

#endif

