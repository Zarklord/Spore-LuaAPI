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

#pragma once

#ifdef LUAAPI_DLL_EXPORT

#include <Spore/BasicIncludes.h>
#include <asmjit/asmjit.h>
#include <tlhelp32.h>
#include <tracy/Tracy.hpp>

class LuaSporeASMJitLogger : public asmjit::Logger {
public:
	ASMJIT_NONCOPYABLE(LuaSporeASMJitLogger)

	LuaSporeASMJitLogger() noexcept
	{
		addFlags(asmjit::FormatFlags::kMachineCode | asmjit::FormatFlags::kHexImms | asmjit::FormatFlags::kHexOffsets);
	}
	~LuaSporeASMJitLogger() noexcept override = default;

	asmjit::Error _log(const char* data, size_t size = SIZE_MAX) noexcept override
	{
		ModAPI::Log(data);
		return asmjit::kErrorOk;
	}
};
inline LuaSporeASMJitLogger sLuaSporeASMJitLogger;

class DetourThreadManager
{
public:
	DetourThreadManager()
	: mPID(GetCurrentProcessId())
	, mMainThreadID(GetCurrentThreadId())
	{	
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(h, &te))
			{
				do {
					if (te.th32OwnerProcessID == mPID)
						AddThread(te.th32ThreadID);

					te.dwSize = sizeof(te);
				} while (Thread32Next(h, &te));
			}
			CloseHandle(h);
		}
	}

	void AddThread(DWORD thread_id)
	{
		if (thread_id == mMainThreadID) return;

		mThreadIDs.push_back(thread_id);
	}

	void RemoveThread(DWORD thread_id)
	{
		const auto end = mThreadIDs.end();
		mThreadIDs.erase(eastl::remove(mThreadIDs.begin(), end, thread_id), end);
	}

	void SetDetourUpdateThreads()
	{
		assert(mMainThreadID == GetCurrentThreadId());
		assert(mThreadHandles.empty());

		mThreadHandles.reserve(mThreadIDs.size());

		for (const DWORD thread_id : mThreadIDs)
		{
			HANDLE h = OpenThread(THREAD_SUSPEND_RESUME, false, thread_id);

			if (h == INVALID_HANDLE_VALUE) continue;
			
			mThreadHandles.push_back(h);
			DetourUpdateThread(h);
		}
	}

	void ClearDetourUpdateThreads()
	{
		assert(mMainThreadID == GetCurrentThreadId());
		for (const HANDLE handle : mThreadHandles)
		{
			CloseHandle(handle);
		}
		mThreadHandles.clear();
	}
private:
	DWORD mPID;
	DWORD mMainThreadID;
	vector<DWORD> mThreadIDs;
	vector<HANDLE> mThreadHandles;
};

inline DetourThreadManager detour_thread_manager;

#endif