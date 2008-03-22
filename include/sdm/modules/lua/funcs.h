/*
 * Header Name:	funcs.h
 * Description:	Lua Functions Header
 */

#ifndef _SDM_MODULES_LUA_FUNCS_H
#define _SDM_MODULES_LUA_FUNCS_H

#include <lua.h>

#include <sdm/globals.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>


int sdm_load_lua_library(lua_State *);

int sdm_lua_do_action(lua_State *);

#endif


