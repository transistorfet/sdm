/*
 * Object Name:	utils.h
 * Description:	Utilities for Things
 */

#ifndef _SDM_THINGS_UTILS_H
#define _SDM_THINGS_UTILS_H

#include <stdio.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/objs/object.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/things/thing.h>
#include <sdm/actions/action.h>


/*** Thing Property Functions ***/

static inline double sdm_get_number_property(struct sdm_thing *thing, const char *name)
{
	struct sdm_number *number;

	if (!(number = SDM_NUMBER(sdm_thing_get_property(thing, name, &sdm_number_obj_type))))
		return(0);
	return(number->num);
}

static inline int sdm_set_number_property(struct sdm_thing *thing, const char *name, double num)
{
	struct sdm_number *number;

	if (!(number = create_sdm_number(num)))
		return(-1);
	return(sdm_thing_set_property(thing, name, SDM_OBJECT(number)));
}

static inline const char *sdm_get_string_property(struct sdm_thing *thing, const char *name)
{
	struct sdm_string *string;

	if (!(string = SDM_STRING(sdm_thing_get_property(thing, name, &sdm_string_obj_type))))
		return(NULL);
	return(string->str);
}

static inline int sdm_set_string_property(struct sdm_thing *thing, const char *name, const char *str)
{
	struct sdm_string *string;

	if (!(string = create_sdm_string(str)))
		return(-1);
	return(sdm_thing_set_property(thing, name, SDM_OBJECT(string)));
}

/*** Thing Action Functions ***/

#define sdm_notify(thing, caller, ...)	\
	( sdm_do_format_action((thing), (caller), "notify", __VA_ARGS__) )

#define sdm_announce(room, caller, ...)	\
	( sdm_do_format_action((room), (caller), "announce", __VA_ARGS__) )

static inline int sdm_do_nil_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action)
{
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.obj = thing;
	args.text = "";
	return(sdm_thing_do_action(thing, action, &args));
}

static inline int sdm_do_format_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, const char *fmt, ...)
{
	int i;
	va_list va;
	char buffer[STRING_SIZE];
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = buffer;

	va_start(va, fmt);
	if ((i = vsnprintf(buffer, STRING_SIZE - 1, fmt, va)) < 0)
		return(-1);
	if (i >= STRING_SIZE - 1)
		buffer[STRING_SIZE - 1] = '\0';
	sdm_thing_do_action(thing, action, &args);
	return(0);
}

/*** Various? ***/

#define sdm_is_user(thing)	\
	( sdm_object_is_a(SDM_OBJECT(thing), &sdm_user_obj_type) )

// TODO this is wrong, sdm_thing_is_a requires the id, not a pointer
#define sdm_is_exit(thing)	\
	( sdm_thing_is_a(SDM_THING(thing), sdm_interpreter_find_thing(NULL, "/core/exit")) )

static inline int sdm_is_mobile(struct sdm_thing *thing)
{

	return(0);
}

static inline int sdm_is_room(struct sdm_thing *thing)
{

	return(0);
}


static inline int sdm_moveto(struct sdm_thing *thing, struct sdm_thing *to, struct sdm_thing *via)
{
	// TODO move the given thing to the given thing via the given exit.
	//	if 'via' is null then treat it like a teleportation

	/** If the 'on_exit' action returns an error, then the object should not be removed */
	if (sdm_do_nil_action(thing->location, thing, "do_leave") < 0)
		return(-1);
	if (sdm_thing_add(to, thing) < 0)
		return(-1);
	// TODO if the add fails, should we do 'on_enter' on the original location?
	/** If the 'on_enter' action returns an error, then the object should not be added */
	if (sdm_do_nil_action(to, thing, "do_enter") < 0)
		return(-1);
	return(0);
}


#endif

