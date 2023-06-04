#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include "LuaConsole.h"
#include <LuaSpore/LuaSpore.h>

using namespace LuaConsole;

virtual_detour(ProcessLine__detour, App::cCheatManager, App::ICheatManager, bool(const char*))
{
	bool detoured(const char* pString)
	{
		bool success = false;

		const auto& lua_spore = GetLuaSpore();
		auto* L = lua_spore.GetLuaState();
		if (sExecuteCheatCommand != LUA_NOREF && sExecuteCheatCommand != LUA_REFNIL)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, sExecuteCheatCommand);
			success = luaL_loadbufferx(L, pString, strlen(pString), nullptr, "t") == LUA_OK;
			if (success)
			{
				success = lua_spore.CallLuaFunction(1, 0);
			}
		}

		if (!success)
		{
			success = original_function(this, pString);
		}

		return success;
	}

	static inline int sExecuteCheatCommand = LUA_NOREF;
};

void LuaConsole::AttachDetours()
{
	ProcessLine__detour::attach(GetAddress(App::cCheatManager, ProcessLine));
}

void LuaConsole::PostInit()
{
	auto* L = GetLuaSpore().GetLuaState();
	lua_getglobal(L, "ExecuteCheatCommand");	
	ProcessLine__detour::sExecuteCheatCommand = luaL_ref(L, LUA_REGISTRYINDEX);
}

#endif