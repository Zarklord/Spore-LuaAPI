#pragma once

#include <sol/sol.hpp>
#include <LuaSpore/LuaInternal.h>
#include <LuaSpore/Bindings.h>
#include <LuaSpore/Extensions/Property.h>

namespace LuaAPI
{
	class LuaPropertyArray
	{
	public:
		LUAAPI LuaPropertyArray(Extensions::Property* prop);
		LUAAPI LuaPropertyArray(const App::PropertyType prop_type, size_t array_size);
		LUAAPI ~LuaPropertyArray();

		LUAAPI void SetProperty(App::Property* prop) const;
	private:
		void FreeArray() const;
		sol::object LuaIndex(sol::this_state L, size_t index) const;
		void LuaNewIndex(size_t index, sol::stack_object value) const;

		static constexpr std::string_view LuaClassName = "PropertyArray";
		friend void LuaAPI::RegisterLuaPropertyArray(sol::state_view& s);
	private:
		App::PropertyType mType;
		uint8_t* mArray;
		size_t mSize;
		size_t mTypeSize;
		bool mReadOnly;
	};
}
