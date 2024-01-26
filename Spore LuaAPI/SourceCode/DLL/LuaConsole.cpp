/****************************************************************************
* Copyright (C) 2023-2024 Zarklord
*
* This file is part of Spore LuaAPI.
*
* Spore LuaAPI is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Spore LuaAPI.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include "pch.h"
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <LuaSpore/SporeDetours.h>
#include <LuaSpore/LuaSporeCallbacks.h>

virtual_detour(ProcessLine, App::cCheatManager, App::ICheatManager, bool(const char*))
{
	bool detoured(const char* pString)
	{
		LUA_THREAD_SAFETY();

		bool success = false;

		const auto& lua_spore = GetLuaSpore();
		auto s = lua_spore.GetState();
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

AddSporeDetours()
{
	ProcessLine::attach(GetAddress(App::cCheatManager, ProcessLine));
}

OnLuaInit(sol::state_view s)
{
	ProcessLine::sExecuteCheatCommand = s["ExecuteCheatCommand"];
}

OnLuaDispose(sol::state_view s)
{
	ProcessLine::sExecuteCheatCommand.reset();
}

#endif