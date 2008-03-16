/*
 * Name:	module.c
 * Description:	Module Manager
 */

#include <sdm/misc.h>
#include <sdm/data.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/modules/module.h>

#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>

static struct sdm_hash *modules_list = NULL;

int init_module(void)
{
	if (modules_list)
		return(1);
	if (!(modules_list = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, NULL)))
		return(-1);
	return(0);
}

int release_module(void)
{
	if (!modules_list)
		return(1);
	destroy_sdm_hash(modules_list);
	modules_list = NULL;
	return(0);
}

int sdm_module_register(const char *name, struct sdm_module *module)
{
	return(sdm_hash_add(modules_list, name, module));
}

int sdm_module_deregister(const char *name)
{
	return(sdm_hash_remove(modules_list, name));
}



