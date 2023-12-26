#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>

static void LuaPrint(const char* str)
{
	ModAPI::Log(str);
}

static uint32_t LuaHash(const LuaFNVHash& value)
{
	return value;
}

void LuaSpore::LoadLuaGlobals(sol::state& s)
{
	s["LuaPrint"] = LuaPrint;
	s["id"] = LuaHash;
	s["GAMETYPE"] = static_cast<uint32_t>(ModAPI::GetGameType());
	s["LUA_VERSION"] = lua_version(s);
}

#endif