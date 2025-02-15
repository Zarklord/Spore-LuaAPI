/****************************************************************************
* Copyright (C) 2023-2025 Zarklord
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

#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <SourceCode/LuaDetours/DetourHelper.h>

#include "tracy/Tracy.hpp"

#define LUAAPI_MODNAME "SporeLuaAPI"
#define LUAAPI_VERSION 10000

void PreInitialize()
{
#if defined(LUAAPI_DLL_EXPORT) && defined(_DEBUG)
	ModAPI::Log("GhidraAddressOffset: 0x%X - 0x400000", baseAddress);
#endif
	LuaSpore::Initialize();
}

void PostInitialize()
{
	App::AddUpdateFunction([]{ FrameMark; });
	GetLuaSpore().PostInit();
}

void Dispose()
{
	LuaSpore::Finalize();
	tracy::ShutdownProfiler();
}

member_detour(PreInit_detour, std::monostate, int(int, int))
{
	int detoured(int arg_0, int arg_1)
	{
		const int result = original_function(this, arg_0, arg_1);
		PreInitialize();
		return result;
	}
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		tracy::StartupProfiler();
		TracySetProgramName("Spore: Galactic Adventures (Modded)");
		ModAPI::AddPostInitFunction(PostInitialize);
		ModAPI::AddDisposeFunction(Dispose);
		
		DetourTransactionBegin();
		PreInit_detour::attach(GetAddress(App::cAppSystem, PreInit));
		LuaSpore::RegisterCPPMod(LUAAPI_MODNAME, LUAAPI_VERSION);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
		detour_thread_manager.AddThread(GetCurrentThreadId());
		break;
	case DLL_THREAD_DETACH:
		detour_thread_manager.RemoveThread(GetCurrentThreadId());
		break;
	}
	return TRUE;
}

#endif