#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>

void LuaPrint(const char* str)
{
	ModAPI::Log(str);
}

uint32_t LuaHash(const char* str)
{
	return id(str);
}

void LuaSpore::LoadLuaGlobals(sol::state& s)
{
	s["LuaPrint"] = LuaPrint;
	s["id"] = LuaHash;
	s["GAMETYPE"] = static_cast<uint32_t>(ModAPI::GetGameType());
	s["LUA_VERSION"] = lua_version(s);
}

#endif