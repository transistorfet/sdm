/*
 * Name:	funcs.c
 * Description:	SDRL Functions
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/objs/number.h>

#include <sdm/modules/lua/funcs.h>

int sdm_load_lua_library(lua_State *state)
{
	lua_register(state, "create_thing", sdm_lua_create_thing);
	lua_register(state, "destroy_thing", sdm_lua_destroy_thing);

	lua_register(state, "do_action", sdm_lua_do_action);
	lua_register(state, "get_parent", sdm_lua_get_parent);
	lua_register(state, "get_location", sdm_lua_get_location);
	lua_register(state, "get_property", sdm_lua_get_property);
	lua_register(state, "set_property", sdm_lua_set_property);

	lua_register(state, "moveto", sdm_lua_moveto);
	lua_register(state, "tell", sdm_lua_tell);
	return(0);
}


/**
 * Args:	<parent>
 * Description:	Creates a new thing and returns the id.
 */
int sdm_lua_create_thing(lua_State *state)
{
	struct sdm_thing *thing, *parent;

	parent = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	if (!(thing = SDM_THING(create_sdm_object(SDM_OBJECT(parent)->type, SDM_THING_ARGS(SDM_NEW_ID, parent->id)))))
		luaL_error(state, "Failed to create new thing");
	else
		lua_pushnumber(state, thing->id);
	return(1);
}

/**
 * Args:	<thing>
 * Description:	Destroys the given thing
 */
int sdm_lua_destroy_thing(lua_State *state)
{
	struct sdm_thing *thing;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	destroy_sdm_object(SDM_OBJECT(thing));
	return(0);
}


/**
 * Args:	<thing>, <caller>, <action>, [<target>], [<args>]
 * Description:	Calls the given action name on the given object reference.
 */
int sdm_lua_do_action(lua_State *state)
{
	int nargs, res;
	const char *action, *args = NULL;
	struct sdm_thing *thing, *caller, *target = NULL;

	nargs = lua_gettop(state);
	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	caller = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 2));
	action = luaL_checkstring(state, 3);
	if (nargs >= 4)
		target = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 4));
	if (nargs >= 5)
		args = luaL_checkstring(state, 5);

	res = sdm_thing_do_action(thing, caller, action, target, args);
	lua_pushnumber(state, res);
	return(1);
}

/**
 * Args:	<thing>
 * Description:	Returns the parent of the given object.
 */
int sdm_lua_get_parent(lua_State *state)
{
	struct sdm_thing *thing;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	if (!thing)
		lua_pushnumber(state, -1);
	else
		lua_pushnumber(state, (lua_Number) thing->parent);
	return(1);
}

/**
 * Args:	<thing>
 * Description:	Returns the location of the given object.
 */
int sdm_lua_get_location(lua_State *state)
{
	struct sdm_thing *thing;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	if (!thing || !thing->location)
		lua_pushnumber(state, -1);
	else
		lua_pushnumber(state, (lua_Number) SDM_THING(thing->location)->id);
	return(1);
}

/**
 * Args:	<thing>, <name>
 * Description:	Returns the named property of the given object.
 */
int sdm_lua_get_property(lua_State *state)
{
	const char *name;
	struct sdm_object *obj;
	struct sdm_thing *thing;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	name = luaL_checkstring(state, 2);

	obj = sdm_thing_get_property(thing, name, NULL);
	if (!obj)
		lua_pushnil(state);
	else if (sdm_object_is_a(obj, &sdm_number_obj_type))
		lua_pushnumber(state, SDM_NUMBER(obj)->num);
	else if (sdm_object_is_a(obj, &sdm_string_obj_type))
		lua_pushstring(state, SDM_STRING(obj)->str);
	else
		// TODO for now we'll only deal with numbers and strings
		lua_pushnil(state);
	return(1);
}

/**
 * Args:	<thing>, <name>, <value>
 * Description:	Sets the named property of the given object to the given value.
 */
int sdm_lua_set_property(lua_State *state)
{
	int res, type;
	const char *name;
	struct sdm_thing *thing;
	struct sdm_object *obj = NULL;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	name = luaL_checkstring(state, 2);

	type = lua_type(state, 3);
	if (type == LUA_TNUMBER)
		obj = SDM_OBJECT(create_sdm_number(lua_tonumber(state, 3)));
	else if (type == LUA_TSTRING)
		obj = SDM_OBJECT(create_sdm_string(lua_tostring(state, 3)));
	else
		luaL_argerror(state, 0, "only number and string types supported");
	res = sdm_thing_set_property(thing, name, obj);
	lua_pushnumber(state, res);
	return(1);
}


/**
 * Args:	<thing>, <container>
 * Description:	Moves the given object to the given container
 */
int sdm_lua_moveto(lua_State *state)
{
	int res;
	struct sdm_thing *thing, *container;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	container = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 2));

	res = sdm_thing_add(container, thing);
	lua_pushnumber(state, res);
	return(1);
}

/**
 * Args:	<thing>, <text>
 * Description:	Simpler function for doing the "tell" action on an object
 */
int sdm_lua_tell(lua_State *state)
{
	int res;
	const char *args;
	struct sdm_thing *thing, *caller;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	args = luaL_checkstring(state, 2);
	lua_getglobal(state, "caller");
	caller = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, -1));

	if (!thing || !caller)
		res = -1;
	else
		res = sdm_thing_do_action(thing, caller, "tell", NULL, args);
	lua_pushnumber(state, res);
	return(1);
}


