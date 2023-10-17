#pragma once

#include <sol\sol.hpp>

namespace LuaAPI
{
	void RegisterLuaPropertyArray(sol::state_view& s);
	void RegisterProperty(sol::state_view& s);
	void RegisterPropertyList(sol::state_view& s);
	void RegisterPropertyTypes(sol::state_view& s);
	void RegisterPropManager(sol::state_view& s);

	inline void RegisterSporeBindings(sol::state_view& s)
	{
		RegisterLuaPropertyArray(s);
		RegisterProperty(s);
		RegisterPropertyList(s);
		RegisterPropertyTypes(s);
		RegisterPropManager(s);
	}
}