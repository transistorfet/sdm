/*
 * Header Name:	module.h
 * Description:	Module Manager Header
 */

#ifndef _SDM_MODULES_MODULE_H
#define _SDM_MODULES_MODULE_H

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/objs/thing.h>

struct sdm_module;

typedef int (*sdm_module_read_action_t)(struct sdm_thing *, struct sdm_data_file *);
// TODO maybe you should pass a a module pointer to the set_action function
//typedef int (*sdm_module_write_action_t)(struct sdm_thing *, struct sdm_data_file *);

struct sdm_module {
	sdm_module_read_action_t read_action;
};

int init_module(void);
int release_module(void);

int sdm_module_register(const char *, struct sdm_module *);
int sdm_module_deregister(const char *);
int sdm_module_read_action(const char *, struct sdm_thing *, struct sdm_data_file *);

#endif
