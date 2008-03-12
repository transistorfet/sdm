/*
 * Object Name:	world.c
 * Description:	Root World Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/actionable.h>
#include <sdm/objs/container.h>
#include <sdm/objs/world.h>

struct sdm_object_type sdm_world_obj_type = {
	&sdm_container_obj_type,
	sizeof(struct sdm_world),
	NULL,
	(sdm_object_init_t) sdm_world_init,
	(sdm_object_release_t) sdm_world_release
};

static struct sdm_world *root_world = NULL;

int init_world(void)
{
	if (root_world)
		return(1);
	if (!(root_world = (struct sdm_world *) create_sdm_object(&sdm_world_obj_type)))
		return(-1);
	// TODO should the read and write data calls be in the object init/release with the filename as an arg
	sdm_world_read_data(root_world, "maps/world.xml");
	return(0);
}

int release_world(void)
{
	if (root_world) {
		sdm_world_write_data(root_world, "maps/world.xml");
		destroy_sdm_object(SDM_OBJECT(root_world));
	}
	return(0);
}

int sdm_world_init(struct sdm_world *world, va_list va)
{
	if (sdm_container_init(SDM_CONTAINER(world), va))
		return(-1);
	return(0);
}

void sdm_world_release(struct sdm_world *world)
{
	sdm_container_release(SDM_CONTAINER(world));
}


int sdm_world_read_data(struct sdm_world *world, const char *file)
{
	const char *type;
	struct sdm_data_file *data;

	if (!(sdm_data_file_exists(file)) || !(data = sdm_data_open(file, SDM_DATA_READ, "world")))
		return(-1);
	do {
		if (!(type = sdm_data_read_name(data)))
			break;
		else if (!strcmp(type, "load")) {
			// TODO load the reference file 
		}
		else {
			sdm_container_read_data(SDM_CONTAINER(world), type, data);
		}
	} while (sdm_data_read_next(data));
	sdm_data_close(data);
	return(0);
}

int sdm_world_write_data(struct sdm_world *world, const char *file)
{
	// TODO write the data back to the world file (how will you know when to use seperate files?)
	//	will it be automatic based on type or something or will it be manual with a flag in the
	//	object saying that it should be written to it's own file?
	return(-1);
}


struct sdm_world *sdm_world_get_root(void)
{
	return(root_world);
}


