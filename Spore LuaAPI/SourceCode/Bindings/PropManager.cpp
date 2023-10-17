#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\Bindings.h>

using namespace LuaAPI;

void LuaAPI::RegisterPropManager(sol::state_view& s)
{
	s.new_usertype<App::IPropManager>(
		"IPropManager",
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
		}
	);
	s["PropManager"] = App::IPropManager::Get();
}

#endif