#pragma once

#ifdef LUAAPI_DLL_EXPORT
#define LUAAPI __declspec(dllexport)
#define LUA_INTERNALPUBLIC public
#else
#define LUAAPI __declspec(dllimport)
#define LUA_INTERNALPUBLIC private
#endif

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