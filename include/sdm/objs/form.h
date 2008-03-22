/*
 * Object Name:	form.h
 * Description:	Form Object
 */

#ifndef _SDM_OBJS_FORM_H
#define _SDM_OBJS_FORM_H

#include <stdarg.h>

#include <sdm/hash.h>
#include <sdm/string.h>
#include <sdm/objs/object.h>
#include <sdm/objs/processor.h>

#define SDM_FORM(ptr)		( (struct sdm_form *) (ptr) )

struct sdm_form {
	struct sdm_processor processor;
	string_t filename;
	// TODO should you load the whole file here or step through the file as you process it?
};

extern struct sdm_processor_type sdm_form_obj_type;

int sdm_form_init(struct sdm_form *, va_list);
void sdm_form_release(struct sdm_form *);

int sdm_form_startup(struct sdm_form *, struct sdm_user *);
int sdm_form_process(struct sdm_form *, struct sdm_user *, char *);
int sdm_form_shutdown(struct sdm_form *, struct sdm_user *);

#endif

