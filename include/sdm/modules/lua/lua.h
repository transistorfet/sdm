/*
 * Header Name:	lua.h
 * Description:	Lua Scripting Module Header
 */

#ifndef _SDM_MODULES_LUA_LUA_H
#define _SDM_MODULES_LUA_LUA_H

#include <lua/lua.h>

#include <sdm/globals.h>
#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/modules/sdrl/funcs.h>
#include <sdm/modules/sdrl/object.h>

#define SDM_LUA(ptr)		( (struct sdm_lua *) (ptr) )

struct sdm_lua {
	struct sdm_action action;

};

extern struct sdm_object_type sdm_lua_obj_type;

int init_lua(void);
void release_lua(void);

int sdm_lua_read_entry(struct sdm_lua *, const char *, struct sdm_data_file *);
int sdm_lua_write_data(struct sdm_lua *, struct sdm_data_file *);

int sdm_lua_action(struct sdrl_expr *, struct sdm_thing *, struct sdm_thing *, struct sdm_thing *, const char *);

#endif


