#pragma once

#ifdef LUAAPI_DLL_EXPORT
#define LUAAPI __declspec(dllexport)
#else
#define LUAAPI __declspec(dllimport)
#endif