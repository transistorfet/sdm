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

struct sdm_module {

};

int init_module(void);
int release_module(void);

int sdm_module_register(const char *, struct sdm_module *);
int sdm_module_deregister(const char *);

#endif
