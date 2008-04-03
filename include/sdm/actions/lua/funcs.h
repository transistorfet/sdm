/*
 * Header Name:	funcs.h
 * Description:	Lua Functions Header
 */

#ifndef _SDM_ACTIONS_LUA_FUNCS_H
#define _SDM_ACTIONS_LUA_FUNCS_H

#include <lua.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>

int sdm_load_lua_library(lua_State *);

int sdm_lua_create_thing(lua_State *);
int sdm_lua_destroy_thing(lua_State *);
int sdm_lua_get_string(lua_State *);
int sdm_lua_get_thing(lua_State *);

int sdm_lua_do_action(lua_State *);
int sdm_lua_get_parent(lua_State *);
int sdm_lua_get_location(lua_State *);
int sdm_lua_get_property(lua_State *);
int sdm_lua_set_property(lua_State *);

int sdm_lua_moveto(lua_State *);
int sdm_lua_notify(lua_State *);


int sdm_lua_convert_object(lua_State *, struct sdm_object *);
struct sdm_object *sdm_lua_convert_lua_value(lua_State *, int);

#endif


