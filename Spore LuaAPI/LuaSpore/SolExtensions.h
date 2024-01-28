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

#pragma once

#include "sol/sol.hpp"

//a bunch of boiler plate so we can use eastl types
template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<sol::string_view>(L, lua_absindex(L, index), handler);
}

inline eastl::string sol_lua_get(sol::types<eastl::string>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	const auto str = sol::stack::get<sol::string_view>(L, lua_absindex(L, index));
	return eastl::string{str.data(), str.length()};
}

inline int sol_lua_push(sol::types<eastl::string>, lua_State* L, const eastl::string& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string16>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<sol::u16string_view>(L, lua_absindex(L, index), handler);
}

inline eastl::string16 sol_lua_get(sol::types<eastl::string16>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	const auto str = sol::stack::get<sol::u16string_view>(L, lua_absindex(L, index));
	return eastl::string16{str.data(), str.length()};
}

inline int sol_lua_push(lua_State* L, const eastl::string16& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string32>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<sol::u32string_view>(L, lua_absindex(L, index), handler);
}

inline eastl::string32 sol_lua_get(sol::types<eastl::string32>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	const auto str = sol::stack::get<sol::u32string_view>(L, lua_absindex(L, index));
	return eastl::string32{str.data(), str.length()};
}

inline int sol_lua_push(lua_State* L, const eastl::string32& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::wstring>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<sol::wstring_view>(L, lua_absindex(L, index), handler);
}

inline eastl::wstring sol_lua_get(sol::types<eastl::wstring>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	const auto str = sol::stack::get<sol::wstring_view>(L, lua_absindex(L, index));
	return eastl::wstring{str.data(), str.length()};
}

inline int sol_lua_push(lua_State* L, const eastl::wstring& str)
{
	return sol::stack::push(L, str.c_str());
}


constexpr uint32_t id(sol::string_view pStr)
{
	uint32_t rez = 0x811C9DC5u;
	for (const char c : pStr)
	{
		// To avoid compiler warnings
		rez = static_cast<uint32_t>(rez * static_cast<unsigned long long>(0x1000193));
		rez ^= static_cast<uint32_t>(const_tolower(c));
	}
	return rez;
}

constexpr uint32_t id(sol::u16string_view pStr)
{
	uint32_t rez = 0x811C9DC5u;
	for (const char16_t c : pStr)
	{
		// To avoid compiler warnings
		rez = static_cast<uint32_t>(rez * static_cast<unsigned long long>(0x1000193));
		rez ^= static_cast<uint32_t>(const_tolower(c));
	}
	return rez;
}

class LuaFNVHash
{
public:
	LuaFNVHash()
	: mHash(0)
	{
	}
	explicit LuaFNVHash(uint32_t hash)
	: mHash(hash)
	{
	}
	explicit LuaFNVHash(const char* hash)
	: mHash(id(hash))
	{
	}
	explicit LuaFNVHash(const char16_t* hash)
	: mHash(id(hash))
	{
	}
	explicit LuaFNVHash(sol::string_view hash)
	: mHash(id(hash))
	{
	}
	explicit LuaFNVHash(sol::u16string_view hash)
	: mHash(id(hash))
	{
	}
	LuaFNVHash& operator=(uint32_t hash)
	{
		mHash = hash;
		return *this; 
	}
	LuaFNVHash& operator=(const char* hash)
	{
		mHash = id(hash);
		return *this; 
	}
	LuaFNVHash& operator=(const char16_t* hash)
	{
		mHash = id(hash);
		return *this; 
	}

	operator uint32_t() const
	{
		return mHash;
	}
	uint32_t get() const
	{
		return mHash;
	}
private:
	uint32_t mHash;
};

template <typename Handler>
inline bool sol_lua_check(sol::types<LuaFNVHash>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	const auto abs_index = lua_absindex(L, index);
	tracking.use(1);
	const bool success = sol::stack::check<uint32_t>(L, abs_index, &sol::no_panic) ||
		sol::stack::check<sol::string_view>(L, abs_index, &sol::no_panic) ||
		sol::stack::check<sol::u16string_view>(L, abs_index, &sol::no_panic);
	if (!success)
	{
		handler(L, index, sol::type::number, sol::type_of(L, abs_index), "[expected number or fnv_hashable type]");
		return false;
	}
	return true;
}

inline LuaFNVHash sol_lua_get(sol::types<LuaFNVHash>, lua_State* L, int index, sol::stack::record& tracking)
{
	const auto abs_index = lua_absindex(L, index);
	tracking.use(1);
	if (const auto char_value = sol::stack::check_get<sol::string_view>(L, abs_index))
	{
		return LuaFNVHash(char_value.value());
	}
	if (const auto char16_value = sol::stack::check_get<sol::u16string_view>(L, abs_index))
	{
		return LuaFNVHash(char16_value.value());
	}
	return LuaFNVHash(sol::stack::get<uint32_t>(L, abs_index));
}

template <typename T>
struct sol::unique_usertype_traits<eastl::intrusive_ptr<T>> {
	typedef T type;
	typedef eastl::intrusive_ptr<T> actual_type;
	static const bool value = true;
	static bool is_null(const actual_type& ptr) {
		return ptr == nullptr;
	}
	static type* get (const actual_type& ptr) {
		return ptr.get();
	}
};