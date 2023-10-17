#pragma once

#include <LuaSpore\LuaInternal.h>

namespace LuaAPI
{
	typedef void(*LuaFunction)(lua_State* L);

	extern LUAAPI void AddLuaInitFunction(LuaFunction);
	extern LUAAPI void AddLuaDisposeFunction(LuaFunction);
}