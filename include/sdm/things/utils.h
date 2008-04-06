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
#include <sdm/processors/interpreter.h>


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

static inline int sdm_do_text_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, const char *str)
{
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.text = str;
	return(sdm_thing_do_action(thing, action, &args));
}

static inline int sdm_do_object_action(struct sdm_thing *thing, struct sdm_thing *caller, const char *action, struct sdm_thing *obj, struct sdm_thing *target)
{
	struct sdm_action_args args;

	memset(&args, '\0', sizeof(struct sdm_action_args));
	args.caller = caller;
	args.obj = obj;
	args.target = target;
	args.text = "";
	return(sdm_thing_do_action(thing, action, &args));
}

/*** Various? ***/

#define sdm_is_user(thing)	\
	( sdm_object_is_a(SDM_OBJECT(thing), &sdm_user_obj_type) )

#define sdm_is_mobile(thing)	\
	( sdm_named_is_a(SDM_THING(thing), "/core/mobile") )

#define sdm_is_exit(thing)	\
	( sdm_named_is_a(SDM_THING(thing), "/core/exit") )

#define sdm_is_room(thing)	\
	( sdm_named_is_a(SDM_THING(thing), "/core/room") )

static inline int sdm_named_is_a(struct sdm_thing *thing, const char *name)
{
	struct sdm_thing *parent;

	if (!(parent = sdm_interpreter_find_thing(NULL, name)))
		return(0);
	return(sdm_thing_is_a(thing, parent->id));
}


static inline int sdm_moveto(struct sdm_thing *caller, struct sdm_thing *thing, struct sdm_thing *to, struct sdm_thing *via)
{
	// TODO move the given thing to the given thing via the given exit.
	//	if 'via' is null then treat it like a teleportation

	// TODO should this take and pass the caller as a seperate value or at the very least pass
	//	the object moving as the obj instead of the caller
	/** If the 'on_exit' action returns an error, then the object should not be removed */
	if (sdm_do_object_action(thing->location, caller, "do_leave", thing, via) < 0)
		return(-1);
	if (sdm_thing_add(to, thing) < 0)
		return(-1);
	// TODO if the add fails, should we do 'on_enter' on the original location?
	/** If the 'on_enter' action returns an error, then the object should not be added */
	if (sdm_do_object_action(to, caller, "do_enter", thing, via) < 0)
		return(-1);
	return(0);
}


int sdm_do_format_action(struct sdm_thing *, struct sdm_thing *, const char *, const char *, ...);

int sdm_util_expand_str(char *, int, struct sdm_action_args *, const char *);
int sdm_util_expand_reference(char *, int, struct sdm_action_args *, const char *, int *);
int sdm_util_resolve_reference(char *, int, struct sdm_action_args *, const char *);
int sdm_util_escape_char(const char *, char *);
int sdm_util_atoi(const char *, int);

#endif

