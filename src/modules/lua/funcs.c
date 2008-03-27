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
#include <sdm/objs/interpreter.h>

#include <sdm/modules/lua/funcs.h>

int sdm_load_lua_library(lua_State *state)
{
	lua_register(state, "create_thing", sdm_lua_create_thing);
	lua_register(state, "destroy_thing", sdm_lua_destroy_thing);
	lua_register(state, "get_string", sdm_lua_get_string);
	lua_register(state, "get_thing", sdm_lua_get_thing);

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
	if (!parent || !(thing = SDM_THING(create_sdm_object(SDM_OBJECT(parent)->type, SDM_THING_ARGS(SDM_NEW_ID, parent->id)))))
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
 * Args:	<string>
 * Description:	Splits the given string into the first argument string and
 *		the remaining string.
 */
int sdm_lua_get_string(lua_State *state)
{
	int used = 0;
	const char *args;
	char buffer[STRING_SIZE];

	args = luaL_checkstring(state, 1);
	sdm_interpreter_get_string(args, buffer, STRING_SIZE, &used);
	for (; (args[used] == ' ') || (args[used] == '\t'); used++)
		;
	lua_pushstring(state, buffer);
	lua_pushstring(state, &args[used]);
	return(2);
}

/**
 * Args:	<thing>, <string>
 * Description:	Attempts to find the object named in the first argument of the
 *		given string and returns it and the remaining string.
 */
int sdm_lua_get_thing(lua_State *state)
{
	int used = 0;
	const char *args;
	struct sdm_thing *thing, *obj;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	args = luaL_checkstring(state, 2);
	if (!thing)
		luaL_argerror(state, 0, "invalid thing id");
	obj = sdm_interpreter_get_thing(thing, args, &used);
	for (; (args[used] == ' ') || (args[used] == '\t'); used++)
		;
	if (!obj)
		lua_pushnumber(state, -1);
	else
		lua_pushnumber(state, (lua_Number) obj->id);
	lua_pushstring(state, &args[used]);
	return(2);
}


/**
 * Args:	<thing>, <action>, [<target>], [<args>]
 * Description:	Calls the given action name on the given object reference.
 */
int sdm_lua_do_action(lua_State *state)
{
	int nargs, res;
	struct sdm_object *obj;
	const char *action, *args = NULL;
	struct sdm_thing *thing, *caller, *target = NULL;

	nargs = lua_gettop(state);
	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	action = luaL_checkstring(state, 2);
	if (nargs >= 3)
		target = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 3));
	if (nargs >= 4)
		args = luaL_checkstring(state, 4);

	lua_getglobal(state, "caller");
	caller = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, -1));

	res = sdm_thing_do_action(thing, caller, action, target, args, &obj);
	sdm_lua_convert_object(state, obj);
	lua_pushnumber(state, res);
	return(2);
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
	struct sdm_thing *thing;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	name = luaL_checkstring(state, 2);
	if (!thing)
		lua_pushnil(state);
	else
		sdm_lua_convert_object(state, sdm_thing_get_property(thing, name, NULL));
	return(1);
}

/**
 * Args:	<thing>, <name>, <value>
 * Description:	Sets the named property of the given object to the given value.
 */
int sdm_lua_set_property(lua_State *state)
{
	const char *name;
	struct sdm_thing *thing;
	struct sdm_object *obj = NULL;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	name = luaL_checkstring(state, 2);

	if (!(obj = sdm_lua_convert_lua_value(state, 3)))
		luaL_argerror(state, 0, "only number and string types supported");
	if (!thing)
		lua_pushnumber(state, -1);
	else
		lua_pushnumber(state, sdm_thing_set_property(thing, name, obj));
	return(1);
}


/**
 * Args:	<thing>, <container>
 * Description:	Moves the given object to the given container
 */
int sdm_lua_moveto(lua_State *state)
{
	struct sdm_thing *thing, *container;

	thing = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 1));
	container = sdm_thing_lookup_id((sdm_id_t) luaL_checknumber(state, 2));

	if (!thing || !container)
		lua_pushnumber(state, -1);
	else
		lua_pushnumber(state, sdm_thing_add(container, thing));
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
		res = sdm_thing_do_action(thing, caller, "tell", NULL, args, NULL);
	lua_pushnumber(state, res);
	return(1);
}



int sdm_lua_convert_object(lua_State *state, struct sdm_object *obj)
{
	if (!obj)
		lua_pushnil(state);
	else if (sdm_object_is_a(obj, &sdm_number_obj_type))
		lua_pushnumber(state, SDM_NUMBER(obj)->num);
	else if (sdm_object_is_a(obj, &sdm_string_obj_type))
		lua_pushstring(state, SDM_STRING(obj)->str);
	else if (sdm_object_is_a(obj, &sdm_thing_obj_type))
		lua_pushnumber(state, SDM_THING(obj)->id);
	else
		/** The object cannot be passed to lua */
		lua_pushnil(state);
	return(0);
}

struct sdm_object *sdm_lua_convert_lua_value(lua_State *state, int index)
{
	struct sdm_object *obj;

	switch (lua_type(state, index)) {
	    case LUA_TNUMBER:
		obj = SDM_OBJECT(create_sdm_number(lua_tonumber(state, index)));
		break;
	    case LUA_TSTRING:
		obj = SDM_OBJECT(create_sdm_string(lua_tostring(state, index)));
		break;
	    default:
		return(NULL);
	}
	return(obj);
}


