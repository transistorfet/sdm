/*
 * Header Name:	module.h
 * Description:	Module Manager Header
 */

#ifndef _SDM_MODULES_MODULE_H
#define _SDM_MODULES_MODULE_H

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/objs/thing.h>

int init_module(void);
int release_module(void);

int sdm_module_read_alias(struct sdm_thing *, const char *, struct sdm_data_file *);

#endif
