#pragma once

#include "sol/sol.hpp"

//a bunch of boiler plate so we can use eastl types
template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<const char*>(L, lua_absindex(L, index), handler);
}

inline eastl::string sol_lua_get(sol::types<eastl::string>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return eastl::string{sol::stack::get<const char*>(L, lua_absindex(L, index))};
}

inline int sol_lua_push(sol::types<eastl::string>, lua_State* L, const eastl::string& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string16>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<const char16_t*>(L, lua_absindex(L, index), handler);
}

inline eastl::string16 sol_lua_get(sol::types<eastl::string16>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return eastl::string16{sol::stack::get<const char16_t*>(L, lua_absindex(L, index))};
}

inline int sol_lua_push(lua_State* L, const eastl::string16& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::string32>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<const char32_t*>(L, lua_absindex(L, index), handler);
}

inline eastl::string32 sol_lua_get(sol::types<eastl::string32>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return eastl::string32{sol::stack::get<const char32_t*>(L, lua_absindex(L, index))};
}

inline int sol_lua_push(lua_State* L, const eastl::string32& str)
{
	return sol::stack::push(L, str.c_str());
}

template <typename Handler>
inline bool sol_lua_check(sol::types<eastl::wstring>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<const wchar_t*>(L, lua_absindex(L, index), handler);
}

inline eastl::wstring sol_lua_get(sol::types<eastl::wstring>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return eastl::wstring{sol::stack::get<const wchar_t*>(L, lua_absindex(L, index))};
}

inline int sol_lua_push(lua_State* L, const eastl::wstring& str)
{
	return sol::stack::push(L, str.c_str());
}

class LuaFNVHash
{
public:
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
	tracking.use(1);
	return sol::stack::multi_check<uint32_t, const char*, const char16_t*>(L, lua_absindex(L, index), handler);
}

inline LuaFNVHash sol_lua_get(sol::types<LuaFNVHash>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	const auto abs_index = lua_absindex(L, index);
	if (sol::stack::check<const char*>(L, abs_index))
	{
		return LuaFNVHash(sol::stack::get<const char*>(L, abs_index));
	}
	if (sol::stack::check<const char16_t*>(L, abs_index))
	{
		return LuaFNVHash(sol::stack::get<const char16_t*>(L, abs_index));
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