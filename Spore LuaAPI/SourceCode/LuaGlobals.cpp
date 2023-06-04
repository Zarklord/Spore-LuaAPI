#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>

int LuaPrint(lua_State* L)
{
	ModAPI::Log(luaL_checkstring(L, 1));
	return 0;
}

int LuaHash(lua_State* L)
{
	lua_pushinteger(L, id(luaL_checkstring(L, 1)));
	return 1;
}

void LuaSpore::LoadLuaGlobals() const
{
	lua_pushcfunction(mLuaState, LuaPrint);
	lua_setglobal(mLuaState, "LuaPrint");

	lua_pushcfunction(mLuaState, LuaHash);
	lua_setglobal(mLuaState, "id");

	lua_pushinteger(mLuaState, static_cast<uint32_t>(ModAPI::GetGameType()));
	lua_setglobal(mLuaState, "GAMETYPE");

	lua_pushnumber(mLuaState, lua_version(mLuaState));
	lua_setglobal(mLuaState, "LUA_VERSION");
}

#endif