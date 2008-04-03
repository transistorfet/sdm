/*
 * Header Name:	lua.h
 * Description:	Lua Scripting Module Header
 */

#ifndef _SDM_ACTIONS_LUA_LUA_H
#define _SDM_ACTIONS_LUA_LUA_H

#include <lua.h>

#include <sdm/misc.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

#define SDM_LUA(ptr)		( (struct sdm_lua *) (ptr) )

struct sdm_lua {
	struct sdm_action action;
	string_t code;
};

extern struct sdm_object_type sdm_lua_obj_type;

int init_lua(void);
void release_lua(void);

void sdm_lua_release(struct sdm_lua *);
int sdm_lua_read_entry(struct sdm_lua *, const char *, struct sdm_data_file *);
int sdm_lua_write_data(struct sdm_lua *, struct sdm_data_file *);

int sdm_lua_action(struct sdm_lua *, struct sdm_thing *, struct sdm_action_args *);

#endif


