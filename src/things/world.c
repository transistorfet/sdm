/*
 * Object Name:	world.c
 * Description:	Root World Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/misc.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/world.h>

struct sdm_object_type sdm_world_obj_type = {
	&sdm_thing_obj_type,
	"world",
	sizeof(struct sdm_world),
	NULL,
	(sdm_object_init_t) sdm_world_init,
	(sdm_object_release_t) sdm_world_release,
	(sdm_object_read_entry_t) sdm_world_read_entry,
	NULL
};

static struct sdm_world *root_world = NULL;

int init_world(void)
{
	if (root_world)
		return(1);
	/** Create the world object with ID = 1, and parent ID = -1 since the world has no parent */
	if (!(root_world = (struct sdm_world *) create_sdm_object(&sdm_world_obj_type, 3, SDM_WORLD_ARGS("maps/world.xml", 1, -1))))
		return(-1);
	if (sdm_object_register_type(&sdm_world_obj_type) < 0)
		return(-1);
	return(0);
}

void release_world(void)
{
	if (!root_world)
		return;
	/** Write the world to disk */
	sdm_world_write(root_world, NULL);
	destroy_sdm_object(SDM_OBJECT(root_world));
	sdm_object_deregister_type(&sdm_world_obj_type);
}

int sdm_world_init(struct sdm_world *world, int nargs, va_list va)
{
	const char *filename;

	if (nargs > 0) {
		filename = va_arg(va, const char *);
		if (!(world->filename = make_string("%s", filename)))
			return(-1);
		nargs--;
	}
	if (sdm_thing_init(SDM_THING(world), nargs, va))
		return(-1);
	sdm_object_read_file(SDM_OBJECT(world), world->filename, "world");
	return(0);
}

void sdm_world_release(struct sdm_world *world)
{
	/** We don't write the world here because we assume the root world is being destroyed and we
	    don't want to write each world twice so if for some reason a world is being destroyed without
	    the root world being destroyed, it must be written manually */
	if (world->filename)
		memory_free(world->filename);
	sdm_thing_release(SDM_THING(world));
}

int sdm_world_read_entry(struct sdm_world *world, const char *type, struct sdm_data_file *data)
{
	struct sdm_object *obj;
	char buffer[STRING_SIZE];

	if (!strcmp(type, "load")) {
		sdm_data_read_attrib(data, "ref", buffer, STRING_SIZE);
		if (!(obj = create_sdm_world(buffer, SDM_NO_ID, 0)))
			return(-1);
		if (sdm_thing_add(SDM_THING(world), SDM_THING(obj)) < 0) {
			destroy_sdm_object(obj);
			return(-1);
		}
	}
	else
		return(SDM_NOT_HANDLED);
	return(SDM_HANDLED);
}


int sdm_world_write(struct sdm_world *world, struct sdm_data_file *data)
{
	int res;

	res = sdm_object_write_file(SDM_OBJECT(world), world->filename, "world");
	if (data) {
		sdm_data_write_begin_entry(data, "load");
		sdm_data_write_attrib(data, "ref", world->filename);
		sdm_data_write_end_entry(data);
	}
	return(res);
}

struct sdm_world *sdm_world_get_root(void)
{
	return(root_world);
}



