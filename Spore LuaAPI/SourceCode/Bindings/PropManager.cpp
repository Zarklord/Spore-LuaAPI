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

#include <LuaSpore/LuaSporeCallbacks.h>

OnLuaInit(sol::state_view s, bool is_main_state)
{
	s.new_usertype<App::IPropManager>(
		"IPropManager",
		"GetPropertyListIds", [](App::IPropManager& prop_manager, const LuaFNVHash& groupID)
		{
			eastl::vector<uint32_t> ids;
			prop_manager.GetPropertyListIDs(groupID, ids);
			return sol::as_table(ids);
		},
		"HasPropertyList", [](App::IPropManager& prop_manager, const LuaFNVHash& instanceID, const LuaFNVHash& groupID)
		{
			return prop_manager.HasPropertyList(instanceID, groupID);
		},
		"GetPropertyList", [](sol::this_state L, App::IPropManager& prop_manager, const LuaFNVHash& instanceID, const LuaFNVHash& groupID) -> sol::object
		{
			const sol::state_view s(L);
			PropertyListPtr property_list;
			if (!prop_manager.GetPropertyList(instanceID, groupID, property_list))
			{
				return sol::nil;
			}
			return sol::make_object(s, property_list);
		},
		"GetGlobalPropertyList", [](sol::this_state L, App::IPropManager& prop_manager, const LuaFNVHash& instanceID) -> sol::object
		{
			const sol::state_view s(L);
			PropertyListPtr property_list;
			if (!prop_manager.GetGlobalPropertyList(instanceID, property_list))
			{
				return sol::nil;
			}
			return sol::make_object(s, property_list);
		},
		sol::meta_function::to_string, [](const App::IPropManager& prop_manager)
		{
			return string().sprintf("App::IPropManager (%p)", &prop_manager);
		}
	);
}

#endif