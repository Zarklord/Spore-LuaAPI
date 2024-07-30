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
	s.new_usertype<App::PropertyList>(
		"PropertyList",
		sol::call_constructor, sol::factories([]()
		{
			return PropertyListPtr(new App::PropertyList);
		}),
		"SetProperty", [](App::PropertyList& prop_list, const LuaFNVHash property_id, const App::Property* value)
		{
			prop_list.SetProperty(property_id, value);
		},
		"RemoveProperty", [](App::PropertyList& prop_list, const LuaFNVHash property_id)
		{
			return prop_list.RemoveProperty(property_id);
		},
		"HasProperty", [](const App::PropertyList& prop_list, const LuaFNVHash property_id)
		{
			return prop_list.HasProperty(property_id);
		},
		"GetProperty", [](const sol::this_state L, const App::PropertyList& prop_list, const LuaFNVHash property_id) -> sol::object
		{
			App::Property* prop;
			if (!prop_list.GetProperty(property_id, prop))
			{
				return sol::nil;
			}
			return sol::make_object(L, prop);
		},
		"CopyFrom", &App::PropertyList::CopyFrom,
		"AddPropertiesFrom", &App::PropertyList::AddPropertiesFrom,
		"CopyAllPropertiesFrom", &App::PropertyList::CopyAllPropertiesFrom,
		"AddAllPropertiesFrom", &App::PropertyList::AddAllPropertiesFrom,
		"GetPropertyIDs", [](sol::this_state L, const App::PropertyList& prop_list)
		{
			eastl::vector<uint32_t> property_ids;
			prop_list.GetPropertyIDs(property_ids);
			return property_ids;
		},
		"Clear",&App::PropertyList::Clear,
		"SetParent", &App::PropertyList::SetParent,
		"SetResourceKey", &App::PropertyList::SetResourceKey,
		sol::meta_function::to_string, [](const App::PropertyList& prop_list)
		{
			return string().sprintf("App::PropertyList (%p)", &prop_list);
		}
	);
}

#endif