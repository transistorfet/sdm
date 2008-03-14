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
#include <sdm/objs/container.h>
#include <sdm/modules/module.h>
#include <sdm/modules/basic.h>

struct sdm_module sdm_basic_module = {
	(sdm_module_read_action_t) sdm_basic_read_action
};

static struct sdm_hash *basic_actions = NULL;

int init_basic(void)
{
	if (basic_actions)
		return(1);
	if (!(basic_actions = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, NULL)))
		return(-1);
	if (sdm_module_register("basic", &sdm_basic_module) < 0)
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
	sdm_module_deregister("basic");
	return(0);
}

int sdm_basic_read_action(struct sdm_thing *thing, struct sdm_data_file *data)
{
	sdm_action_t action;
	char buffer[STRING_SIZE];

	if (sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		return(-1);
	if (!(action = sdm_hash_find(basic_actions, buffer)))
		return(-1);
	if (sdm_data_read_attrib(data, "name", buffer, STRING_SIZE) < 0)
		return(-1);
	return(sdm_thing_set_action(thing, buffer, action, NULL, NULL));
}


int sdm_basic_action_look(void *ptr, struct sdm_user *user, struct sdm_thing *thing, const char *args)
{
	struct sdm_string *obj;

	if ((obj = SDM_STRING(sdm_thing_get_property(thing, "name", &sdm_string_obj_type))))
		sdm_user_tell(user, "%s\n", obj->str);
	if (!(obj = SDM_STRING(sdm_thing_get_property(thing, "description", &sdm_string_obj_type))))
		return(-1);
	sdm_user_tell(user, "%s\n", obj->str);
	return(0);
}

int sdm_basic_action_move(void *ptr, struct sdm_user *user, struct sdm_thing *thing, const char *args)
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

