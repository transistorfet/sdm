/*
 * Name:	builtin.c
 * Description:	Builtin Stuff
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/actions/action.h>
#include <sdm/actions/builtin/basics.h>
#include <sdm/actions/builtin/builtin.h>

struct sdm_object_type sdm_builtin_obj_type = {
	&sdm_action_obj_type,
	"builtin",
	sizeof(struct sdm_builtin),
	NULL,
	(sdm_object_init_t) NULL,
	(sdm_object_release_t) NULL,
	(sdm_object_read_entry_t) sdm_builtin_read_entry,
	(sdm_object_write_data_t) sdm_builtin_write_data
};

static struct sdm_hash *builtin_actions = NULL;

extern int sdm_builtin_load_builder(struct sdm_hash *);
extern int sdm_builtin_load_room_actions(struct sdm_hash *);

int init_builtin(void)
{
	if (builtin_actions)
		return(1);
	if (!(builtin_actions = create_sdm_hash(0, -1, NULL)))
		return(-1);
	if (sdm_object_register_type(&sdm_builtin_obj_type) < 0)
		return(-1);
	sdm_builtin_load_basics(builtin_actions);
	sdm_builtin_load_builder(builtin_actions);
	sdm_builtin_load_room_actions(builtin_actions);
	return(0);
}

void release_builtin(void)
{
	if (!builtin_actions)
		return;
	destroy_sdm_hash(builtin_actions);
	builtin_actions = NULL;
	sdm_object_deregister_type(&sdm_builtin_obj_type);
}


int sdm_builtin_read_entry(struct sdm_builtin *builtin, const char *name, struct sdm_data_file *data)
{
	char buffer[STRING_SIZE];

	if (sdm_data_read_string(data, buffer, STRING_SIZE) < 0)
		return(-1);
	if (!(builtin->entry = sdm_hash_find_entry(builtin_actions, buffer)))
		return(-1);
	SDM_ACTION(builtin)->func = (sdm_action_t) builtin->entry->data;
	return(SDM_HANDLED_ALL);
}

int sdm_builtin_write_data(struct sdm_builtin *builtin, struct sdm_data_file *data)
{
	sdm_data_write_string(data, builtin->entry->name);
	return(0);
}


