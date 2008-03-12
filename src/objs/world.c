/*
 * Object Name:	world.c
 * Description:	Root World Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/container.h>
#include <sdm/objs/world.h>

struct sdm_object_type sdm_world_obj_type = {
	&sdm_container_obj_type,
	sizeof(struct sdm_world),
	NULL,
	(sdm_object_init_t) sdm_world_init,
	(sdm_object_release_t) sdm_world_release,
	(sdm_object_read_entry_t) sdm_world_read_entry,
	(sdm_object_write_data_t) sdm_world_write_data
};

static struct sdm_world *root_world = NULL;

int init_world(void)
{
	if (root_world)
		return(1);
	if (!(root_world = (struct sdm_world *) create_sdm_object(&sdm_world_obj_type)))
		return(-1);
	// TODO should the read and write data calls be in the object init/release with the filename as an arg
	sdm_object_read_file(SDM_OBJECT(root_world), "maps/world.xml", "world");
	return(0);
}

int release_world(void)
{
	if (root_world) {
		// TODO have the world written on exit when you are comfortable with the system
		//sdm_object_write_file(SDM_OBJECT(root_world), "maps/world.xml", "world");
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

int sdm_world_read_entry(struct sdm_world *world, const char *type, struct sdm_data_file *data)
{
	if (!strcmp(type, "load")) {
		// TODO load the reference file 
	}
	else
		return(0);
	return(1);
}

int sdm_world_write_data(struct sdm_world *world, struct sdm_data_file *data)
{
	// TODO write data
}


struct sdm_world *sdm_world_get_root(void)
{
	return(root_world);
}



