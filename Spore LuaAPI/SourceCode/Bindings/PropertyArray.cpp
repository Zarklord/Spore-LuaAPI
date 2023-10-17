#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\Bindings.h>
#include <LuaSpore\Types\LuaPropertyArray.h>

void LuaAPI::RegisterLuaPropertyArray(sol::state_view& s)
{
	s.new_usertype<LuaPropertyArray>(
		"PropertyArray",
		sol::constructors<LuaPropertyArray(App::PropertyType, size_t)>(),
		
		sol::meta_function::index, &LuaPropertyArray::LuaIndex,
		sol::meta_function::new_index, &LuaPropertyArray::LuaNewIndex
	);
}

#endif