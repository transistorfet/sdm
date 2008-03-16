/*
 * Name:	basic.c
 * Description:	Basic Stuff...
 */

#include <stdio.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/objs/action.h>
#include <sdm/objs/container.h>
#include <sdm/modules/module.h>
#include <sdm/modules/basic/basic.h>

struct sdm_object_type sdm_basic_obj_type = {
	&sdm_action_obj_type,
	sizeof(struct sdm_basic),
	NULL,
	(sdm_object_init_t) NULL,
	(sdm_object_release_t) NULL,
	(sdm_object_read_entry_t) sdm_basic_read_entry,
	(sdm_object_write_data_t) sdm_basic_write_data
};

static struct sdm_hash *basic_actions = NULL;

int init_basic(void)
{
	if (basic_actions)
		return(1);
	if (!(basic_actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, NULL)))
		return(-1);
	if (sdm_object_register_type("basic", &sdm_basic_obj_type) < 0)
		return(-1);

	sdm_hash_add(basic_actions, "basic_look", sdm_basic_action_look);
	sdm_hash_add(basic_actions, "basic_move", sdm_basic_action_move);
	return(0);
}

int release_basic(void)
{
	if (!basic_actions)
		return(1);
	destroy_sdm_hash(basic_actions);
	basic_actions = NULL;
	sdm_object_deregister_type("basic");
	return(0);
}


int sdm_basic_read_entry(struct sdm_basic *basic, const char *name, struct sdm_data_file *data)
{
	sdm_action_t action;
	char buffer[STRING_SIZE];

	if (sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		return(-1);
	if (!(action = sdm_hash_find(basic_actions, buffer)))
		return(-1);
	SDM_ACTION(basic)->func = action;
	return(SDM_HANDLED);
}

int sdm_basic_write_data(struct sdm_basic *basic, struct sdm_data_file *data)
{
	// TODO implement
	return(0);
}


int sdm_basic_action_look(struct sdm_action *action, struct sdm_user *user, struct sdm_thing *thing, const char *args)
{
	struct sdm_thing *cur;
	struct sdm_string *string;

	if ((string = SDM_STRING(sdm_thing_get_property(thing, "name", &sdm_string_obj_type))))
		sdm_user_tell(user, "<brightyellow>%s</brightyellow>\n", string->str);
	if (!(string = SDM_STRING(sdm_thing_get_property(thing, "description", &sdm_string_obj_type))))
		return(-1);
	sdm_user_tell(user, "<brightgreen>%s</brightgreen>\n", string->str);
	if (sdm_object_is_a(SDM_OBJECT(thing), &sdm_container_obj_type)) {
		for (cur = thing; cur; cur = cur->next) {

		}
	}
	return(0);
}

int sdm_basic_action_move(struct sdm_action *action, struct sdm_user *user, struct sdm_thing *thing, const char *args)
{
	struct sdm_number *obj;
	struct sdm_container *target;

	if (!(obj = SDM_NUMBER(sdm_thing_get_property(thing, "target", &sdm_number_obj_type)))
	    || !(target = SDM_CONTAINER(sdm_thing_lookup_id((sdm_id_t) obj->num)))
	    || !sdm_object_is_a(SDM_OBJECT(target), &sdm_container_obj_type))
		return(-1);
	sdm_container_add(target, SDM_THING(user));
	sdm_thing_do_action(SDM_THING(target), user, "look", "");
	return(0);
}


