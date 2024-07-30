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

#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <Spore/App/cPropManager.h>

#include <LuaSpore/SporeDetours.h>
#include <LuaSpore/LuaSporeCallbacks.h>

static sol::function* sOnPropManagerInitialized = nullptr;

OnLuaPostInit(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	sOnPropManagerInitialized = new sol::function(s["OnPropManagerInitialized"]);
}

OnLuaDispose(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	delete sOnPropManagerInitialized;
	sOnPropManagerInitialized = nullptr;
}

virtual_detour(Initialize_detour, App::cPropManager, App::IPropManager, bool())
{
	bool detoured()
	{
		const bool real_initialize = !mbIsInitialized;
		const bool result = original_function(this);

		if (real_initialize && sOnPropManagerInitialized && sOnPropManagerInitialized->valid())
		{
			GetLuaSpore().ExecuteOnAllStates([this](sol::state_view s, bool is_main_state)
			{
				s["PropManager"] = static_cast<App::IPropManager*>(this);
				if (!is_main_state) return;
				std::ignore = sOnPropManagerInitialized->call();
			});
		}
		return result;
	}
};

AddSporeDetours()
{
	Initialize_detour::attach(GetAddress(App::cPropManager, Initialize));
}

#endif