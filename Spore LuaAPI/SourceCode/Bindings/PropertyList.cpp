#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\LuaBinding.h>

AddLuaBinding(sol::state_view s)
{
	s.new_usertype<App::PropertyList>(
		"PropertyList",
		sol::call_constructor, sol::factories([]()
		{
			return PropertyListPtr(new App::PropertyList);
		}),
		"SetProperty", [](App::PropertyList& prop_list, LuaFNVHash property_id, const App::Property* value)
		{
			prop_list.SetProperty(property_id, value);
		},
		"RemoveProperty", [](App::PropertyList& prop_list, LuaFNVHash property_id)
		{
			return prop_list.RemoveProperty(property_id);
		},
		"HasProperty", [](const App::PropertyList& prop_list, LuaFNVHash property_id)
		{
			return prop_list.HasProperty(property_id);
		},
		"GetProperty", [](sol::this_state L, const App::PropertyList& prop_list, LuaFNVHash property_id) -> sol::object
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
		"SetResourceKey", &App::PropertyList::SetResourceKey
	);
}

#endif