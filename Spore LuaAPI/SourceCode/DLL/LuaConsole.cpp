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

static sol::function* sExecuteCheatCommand = nullptr;

OnLuaPostInit(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	sExecuteCheatCommand = new sol::function(s["ExecuteCheatCommand"]);
}

OnLuaDispose(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	delete sExecuteCheatCommand;
	sExecuteCheatCommand = nullptr;
}

virtual_detour(ProcessLine, App::cCheatManager, App::ICheatManager, bool(const char*))
{
	bool detoured(const char* pString)
	{
		if (!sExecuteCheatCommand || !sExecuteCheatCommand->valid())
			return original_function(this, pString);

		ArgScript::Line argscript_line(pString);
		const bool valid_cheat = argscript_line.GetArgumentsCount() > 0 &&
			CALL(Address(ModAPI::ChooseAddress(0x8441E0, 0x8439D0)), ArgScript::IParser*, Args(ArgScript::FormatParser*, const char*), Args(this->GetArgScript(), argscript_line.GetArgumentAt(0))) != nullptr;

		if (valid_cheat)
			return original_function(this, pString);

		eastl::string error_message;
		GetLuaSpore().ExecuteOnMainState([&error_message, pString](sol::state_view s)
		{
			constexpr sol::string_view chunk_name = "CheatConsoleInput";
			constexpr sol::string_view error_prefix = "[string \"CheatConsoleInput\"]:1: ";


			const auto load_result = s.load(sol::string_view(pString), chunk_name.data(), sol::load_mode::text);
			if (!load_result.valid())
			{
				const sol::error err = load_result;
				sol::string_view err_msg = err.what();
				if (err_msg.substr(0, error_prefix.size()) == error_prefix)
				{
					err_msg.remove_prefix(error_prefix.size());
				}
				error_message.assign(err_msg.data(), err_msg.size());
				return;
			}
			
			const auto call_result = sExecuteCheatCommand->call(load_result.get<sol::function>());
			if (!call_result.valid())
			{
				const sol::error err = call_result;
				error_message = err.what();
				return;
			}

			if (sol::optional<sol::string_view> err_msg = call_result)
			{
				if (err_msg->substr(0, error_prefix.size()) == error_prefix)
				{
					err_msg->remove_prefix(error_prefix.size());
				}
				error_message.assign(err_msg->data(), err_msg->size());
			}
		});

		if (error_message.empty())
			return true;

		App::ConsolePrintF("Unknown command\n      in command '%s' or lua error: '%s'", pString, error_message.c_str());
		return false;
	}
};

AddSporeDetours()
{
	ProcessLine::attach(GetAddress(App::cCheatManager, ProcessLine));
}

#endif