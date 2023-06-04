#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#ifdef LUAAPI_DLL_EXPORT
#define LUAAPI __declspec(dllexport)
#else
#define LUAAPI __declspec(dllimport)
#endif