/*
 * Name:	lua.c
 * Description:	Lua Scripting Module
 */

#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>

#include <sdm/actions/lua/lua.h>
#include <sdm/actions/lua/funcs.h>


LuaAction::LuaAction()
{


}

int LuaAction::do_action(Thing *thing, Args *args)
{

}




struct sdm_object_type sdm_lua_obj_type = {
	&sdm_action_obj_type,
	"lua",
	sizeof(struct sdm_lua),
	NULL,
	(sdm_object_init_t) NULL,
	(sdm_object_release_t) sdm_lua_release,
	(sdm_object_read_entry_t) sdm_lua_read_entry,
	(sdm_object_write_data_t) sdm_lua_write_data
};

lua_State *global_state = NULL;

int init_lua(void)
{
	if (global_state)
		return(1);
	if (!(global_state = lua_open()))
		return(-1);

	lua_strlibopen(global_state);
	if (sdm_load_lua_library(global_state))
		return(-1);

	if (sdm_object_register_type(&sdm_lua_obj_type) < 0)
		return(-1);
	return(0);
}

void release_lua(void)
{
	if (!global_state)
		return;
	sdm_object_deregister_type(&sdm_lua_obj_type);
	if (global_state)
		lua_close(global_state);
	global_state = NULL;
}


void sdm_lua_release(struct sdm_lua *action)
{
	if (action->code)
		memory_free(action->code);
}

int sdm_lua_read_entry(struct sdm_lua *action, const char *name, struct sdm_data_file *data)
{
	int res;
	char buffer[LARGE_STRING_SIZE];

	if ((res = sdm_data_read_raw_string(data, buffer, LARGE_STRING_SIZE)) < 0)
		return(-1);
	if (!(action->code = make_string("%s", buffer)))
		return(-1);
	SDM_ACTION(action)->func = (sdm_action_t) sdm_lua_action;
	return(SDM_HANDLED_ALL);
}

int sdm_lua_write_data(struct sdm_lua *action, struct sdm_data_file *data)
{
	sdm_data_write_raw_string(data, action->code);
	return(0);
}


int sdm_lua_action(struct sdm_lua *action, struct sdm_thing *thing, struct sdm_action_args *args)
{
	const char *error;

/*
 * You could maybe make every object a table with the meta table set to a lua table for 'thing' and the id set
 * to the thing's id.  That wont make calling actions easier though...  But at least you could call the thing
 * objects through the type =/
 */

	lua_pushnumber(global_state, args->caller ? args->caller->id : -1);
	lua_setglobal(global_state, "caller");
	lua_pushnumber(global_state, thing ? thing->id : -1);
	lua_setglobal(global_state, "this");
	lua_pushnumber(global_state, args->obj ? args->obj->id : -1);
	lua_setglobal(global_state, "obj");
	lua_pushnumber(global_state, args->target ? args->target->id : -1);
	lua_setglobal(global_state, "target");
	lua_pushstring(global_state, args->text);
	lua_setglobal(global_state, "text");

	if (luaL_loadbuffer(global_state, action->code, strlen(action->code), args->action)
	    || lua_pcall(global_state, 0, 1, 0)) {
		if ((error = lua_tostring(global_state, -1)))
			sdm_status("LUA: %s", error);
		else
			sdm_status("LUA: Unspecified error occurred while executing lua action");
		return(-1);
	}
	// TODO What if an object is returned as a number (id)?
	args->result = sdm_lua_convert_lua_value(global_state, -1);
	return(0);
}


