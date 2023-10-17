#pragma once

#ifdef LUAAPI_DLL_EXPORT
#define LUAAPI __declspec(dllexport)
#define LUA_INTERNALPUBLIC public
#else
#define LUAAPI __declspec(dllimport)
#define LUA_INTERNALPUBLIC private
#endif