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

#include "Spore/App/cPropManager.h"

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/SporeDetours.h>
#include <LuaSpore/LuaSporeCallbacks.h>

virtual_detour(Initialize_detour, App::cPropManager, App::IPropManager, bool())
{
	bool detoured()
	{
		const bool real_initialize = !mbIsInitialized;

		const bool result = original_function(this);

		if (real_initialize)
		{
			LUA_THREAD_SAFETY();
			sol::state_view s = GetLuaSpore().GetState();
			s["PropManager"] = static_cast<App::IPropManager*>(this);
			(void)sOnPropManagerInitialized();
		}
		return result;
	}

	static inline sol::unsafe_function sOnPropManagerInitialized;
};

OnLuaPostInit(sol::state_view s)
{
	Initialize_detour::sOnPropManagerInitialized = s["OnPropManagerInitialized"];
}

OnLuaDispose(sol::state_view s)
{
	Initialize_detour::sOnPropManagerInitialized.reset();
}

AddSporeDetours()
{
	Initialize_detour::attach(GetAddress(App::cPropManager, Initialize));
}

#endif