/*
 * Name:	funcs.c
 * Description:	SDRL Functions
 */

#include <lua.h>

#include <sdm/objs/user.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/objs/number.h>

#include <sdm/modules/lua/funcs.h>

int sdm_load_lua_library(lua_State *state)
{
	/** Bind functions */

	return(0);
}

/**
 * Args:	<thing>, <caller, <action>, [<target>], [<args>]
 * Description:	Calls the given action name on the given object reference.
 */
int sdm_lua_do_action(lua_State *state)
{

}


