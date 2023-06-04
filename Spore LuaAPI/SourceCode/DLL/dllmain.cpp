#include <pch.h>	


#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include "LuaConsole.h"

void Main()
{
}

void Initialize()
{
	ManualBreakpoint();
	LuaSpore::Initialize();
}

void PostInitialize()
{
	LuaSpore& lua_spore = GetLuaSpore();
	lua_spore.PostInit();
	
	LuaConsole::PostInit();
}

void Dispose()
{
	LuaSpore::Finalize();
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