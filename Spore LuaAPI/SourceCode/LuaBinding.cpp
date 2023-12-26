#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\LuaAPI.h>
#include <LuaSpore\LuaBinding.h>

using namespace LuaAPI;

std::vector<LuaFunction> LuaBinding::sLuaBindings;

LuaBinding::LuaBinding(LuaFunction f)
{
	sLuaBindings.push_back(f);	
}

void LuaBinding::ExecuteLuaBindings(lua_State* L)
{
	for (const auto function : sLuaBindings)
		function(L);
}

#endif