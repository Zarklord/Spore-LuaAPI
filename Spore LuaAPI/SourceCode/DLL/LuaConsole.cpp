#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include "LuaConsole.h"
#include <LuaSpore/LuaSpore.h>

using namespace LuaConsole;

virtual_detour(ProcessLine, App::cCheatManager, App::ICheatManager, bool(const char*))
{
	bool detoured(const char* pString)
	{
		bool success = false;

		auto& lua_spore = GetLuaSpore();
		auto& s = lua_spore.GetState();
		if (sExecuteCheatCommand.valid())
		{
			auto result = s.load_buffer(pString, strlen(pString), "CheatConsoleInput", sol::load_mode::text);
			success = result.valid();
			if (success)
			{
				sExecuteCheatCommand(result);
			}
		}

		if (!success)
		{
			success = original_function(this, pString);
		}

		return success;
	}

	static inline sol::function sExecuteCheatCommand;
};

void LuaConsole::AttachDetours()
{
	ProcessLine::attach(GetAddress(App::cCheatManager, ProcessLine));
}

void LuaConsole::LuaInitialize(sol::state_view& s)
{
	ProcessLine::sExecuteCheatCommand = s["ExecuteCheatCommand"];
}

void LuaConsole::LuaDispose(sol::state_view& s)
{
	ProcessLine::sExecuteCheatCommand.reset();
}

#endif