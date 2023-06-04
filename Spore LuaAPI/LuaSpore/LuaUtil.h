#pragma once

#include <LuaSpore\LuaInternal.h>

inline uint32_t lua_checkfnvhash(lua_State* L, int arg)
{
	if (lua_type(L, arg) == LUA_TSTRING)
	{
		const char* str = luaL_checkstring(L, arg);
		return id(str);
	}
	return static_cast<uint32_t>(luaL_checkinteger(L, arg));
}