/*
 * Object Name:	interpreter.h
 * Description:	Command Interpreter Object
 */

#ifndef _SDM_OBJS_INTERPRETER_H
#define _SDM_OBJS_INTERPRETER_H

#include <stdarg.h>

#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>

#define SDM_CMD_CLOSE		1337

#define SDM_INTERPRETER(ptr)		( (struct sdm_interpreter *) (ptr) )

struct sdm_interpreter {
	struct sdm_processor processor;
};

extern struct sdm_processor_type sdm_interpreter_obj_type;

int init_interpreter(void);
void release_interpreter(void);

int sdm_interpreter_startup(struct sdm_interpreter *, struct sdm_user *);
int sdm_interpreter_process(struct sdm_interpreter *, struct sdm_user *, char *);
int sdm_interpreter_shutdown(struct sdm_interpreter *, struct sdm_user *);

int sdm_interpreter_do_command(struct sdm_thing *, const char *, const char *);
int sdm_interpreter_get_string(const char *, char *, int, int *);
struct sdm_thing *sdm_interpreter_get_thing(struct sdm_thing *, const char *, int *);
struct sdm_thing *sdm_interpreter_find_thing(struct sdm_thing *, const char *);

#endif

