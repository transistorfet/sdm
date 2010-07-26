/*
 * Object Name:	form.c
 * Description:	Form Object
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sdm/text.h>
#include <sdm/misc.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/string.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/utils.h>
#include <sdm/interfaces/interface.h>
#include <sdm/processes/interpreter.h>

#include <sdm/objs/object.h>
#include <sdm/processes/process.h>
#include <sdm/processes/form.h>

struct sdm_processor_type sdm_form_obj_type = { {
	(struct sdm_object_type *) &sdm_processor_obj_type,
	"form",
	sizeof(struct sdm_form),
	NULL,
	(sdm_object_init_t) sdm_form_init,
	(sdm_object_release_t) sdm_form_release,
	(sdm_object_read_entry_t) NULL,
	(sdm_object_write_data_t) NULL	},
	(sdm_processor_startup_t) sdm_form_startup,
	(sdm_processor_process_t) sdm_form_process,
	(sdm_processor_shutdown_t) sdm_form_shutdown
};

int sdm_form_init(struct sdm_form *form, int nargs, va_list va)
{
	const char *filename;

	if (nargs <= 0)
		return(0);
	filename = va_arg(va, const char *);
	if (!(form->filename = make_string("%s", filename)))
		return(-1);
	return(0);
}

void sdm_form_release(struct sdm_form *form)
{
	if (form->filename)
		memory_free(form->filename);
}


int sdm_form_startup(struct sdm_form *proc, struct sdm_user *user)
{
	struct sdm_thing *room;

	// TODO process the file instead or at least make it run a script to do this
	sdm_thing_assign_new_id(SDM_THING(user));
	sdm_set_string_property(SDM_THING(user), "name", user->name);

	// TODO hack, remove when you get a chargen process that can set the location
	SDM_THING(user)->parent = 6;
	if ((room = sdm_thing_lookup_id(50)))
		sdm_moveto(SDM_THING(user), SDM_THING(user), room, NULL);

	/** Replace this processor with an interpreter */
	destroy_sdm_object(SDM_OBJECT(user->proc));
	if (!(user->proc = SDM_PROCESSOR(create_sdm_object(SDM_OBJECT_TYPE(&sdm_interpreter_obj_type), 0))))
		return(-1);
	sdm_processor_startup(user->proc, user);
	return(0);
}

int sdm_form_process(struct sdm_form *proc, struct sdm_user *user, char *input)
{

	return(0);
}

int sdm_form_shutdown(struct sdm_form *proc, struct sdm_user *user)
{
	return(0);
}


