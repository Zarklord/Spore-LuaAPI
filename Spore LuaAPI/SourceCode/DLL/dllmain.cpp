#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <LuaSpore/LuaAPI.h>
#include <LuaSpore\LuaBinding.h>
#include <LuaConsole.h>

void Main()
{
}

void Initialize()
{
#if defined(LUAAPI_DLL_EXPORT) && defined(_DEBUG)
	ManualBreakpoint();
#endif
	LuaSpore::Initialize();
}

void PostInitialize()
{
	GetLuaSpore().PostInit();
}

void Dispose()
{
	LuaSpore::Finalize();
}

void LuaInitialize(lua_State* L)
{
	sol::state_view s(L);
	LuaConsole::LuaInitialize(s);
}

void LuaDispose(lua_State* L)
{
	sol::state_view s(L);
	LuaConsole::LuaDispose(s);
}

void AttachDetours()
{
	LuaConsole::AttachDetours();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Main();
		ModAPI::AddPostInitFunction(PostInitialize);
		ModAPI::AddInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);
		LuaAPI::AddLuaInitFunction(LuaInitialize);
		LuaAPI::AddLuaDisposeFunction(LuaDispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

#endif