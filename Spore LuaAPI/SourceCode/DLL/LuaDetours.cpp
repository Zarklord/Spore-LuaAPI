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

#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <LuaSpore/LuaSporeCallbacks.h>

#include <tracy/Tracy.hpp>

#include <asmjit/asmjit.h>
#include <detours.h>
#include <tlhelp32.h>

namespace
{
	#ifdef TRACY_ON_DEMAND
	thread_local uint32_t tracy_counter = 0;
	thread_local bool tracy_active = false;
	#endif

	using namespace tracy;
	void TracyZonePush(const SourceLocationData* srcloc)
	{
#ifdef TRACY_ENABLE
#ifdef TRACY_ON_DEMAND
			const auto zone_count = tracy_counter++;
			if(zone_count != 0 && !tracy_active) return;
			tracy_active = TracyIsConnected;
			if(!tracy_active) return;
#endif
			TracyQueuePrepare(QueueType::ZoneBegin)
			MemWrite(&item->zoneBegin.time, Profiler::GetTime());
			MemWrite(&item->zoneBegin.srcloc, srcloc);
			TracyQueueCommit(zoneBeginThread);
#endif
	}

	using namespace tracy;
	void TracyZonePop()
	{
#ifdef TRACY_ENABLE
#ifdef TRACY_ON_DEMAND
		assert(tracy_counter != 0);
		tracy_counter--;
		if(!tracy_active) return;
		if(!TracyIsConnected)
		{
			tracy_active = false;
			return;
		}
#endif
		TracyQueuePrepare(QueueType::ZoneEnd);
		MemWrite(&item->zoneEnd.time, Profiler::GetTime());
		TracyQueueCommit(zoneEndThread);
#endif
	}
}

using namespace asmjit;

class ModAPILogger : public Logger {
public:
	ASMJIT_NONCOPYABLE(ModAPILogger)

	ModAPILogger() noexcept
	{
		addFlags(FormatFlags::kMachineCode | FormatFlags::kHexImms | FormatFlags::kHexOffsets);
	}
	~ModAPILogger() noexcept override = default;

	Error _log(const char* data, size_t size = SIZE_MAX) noexcept override
	{
		ModAPI::Log(data);
		return kErrorOk;
	}
};
static ModAPILogger mod_api_logger;

struct LuaDetourFunctionInfo
{
	uintptr_t disk_address{};
	uintptr_t address{};

	void* GetFunctionPointer() const
	{
		return reinterpret_cast<void*>(Address(ModAPI::ChooseAddress(disk_address, address))); //NOLINT
	}
};

class LuaDetour
{
public:
	LuaDetour(LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn);

	~LuaDetour();

	LuaDetour(const LuaDetour&) = delete;
	LuaDetour& operator=(const LuaDetour&) = delete;
	LuaDetour(LuaDetour&&) = delete;
	LuaDetour& operator=(LuaDetour&&) = delete;

	void ExecuteLuaPreFunction();
	void ExecuteLuaPostFunction();

private:
	LuaDetourFunctionInfo mFunctionInfo{};
	
	LuaMultiReference<sol::function> mLuaPreFunction{};
	LuaMultiReference<sol::function> mLuaPostFunction{};

	size_t mNumLuaDetourCallInstances = 0;

	JitRuntime mRuntime;

	void* mOriginalFunction = nullptr;
	void* mDetourPreFunction = nullptr;
	void* mDetourPostFunction = nullptr;
};

namespace
{
	void SaveASMRegisters(x86::Assembler& a)
	{
		a.push(x86::eax);
		a.push(x86::ecx);
		a.push(x86::edx);
	}
	void LoadASMRegisters(x86::Assembler& a)
	{
		a.pop(x86::edx);
		a.pop(x86::ecx);
		a.pop(x86::eax);
	}

	void* AddToRuntime(CodeHolder& code, JitRuntime& runtime)
	{
		void* func = nullptr;
		runtime._add(&func, &code);
		code.reset();
		return func;
	}

	uint32_t* AllocPreservationData()
	{
		static constexpr SourceLocationData srcloc { "DetourFunctionRunLuaCallback", TracyFunction,  TracyFile, (uint32_t)TracyLine, 0 };
		TracyZonePush(&srcloc);
		return new uint32_t[2];
	}

	void FreePreservationData(const uint32_t* data)
	{
		TracyZonePop();
		delete[] data;
	}
	
	struct DetourFunctionRunLuaCallback
	{
		static void* Get(JitRuntime& runtime, bool has_pre, bool has_post, void* this_ptr, void** original_function_ptr)
		{
			thread_local CodeHolder code;
			code.init(runtime.environment(), runtime.cpuFeatures());
			code.setLogger(&mod_api_logger);
			x86::Assembler a(&code);

			SaveASMRegisters(a);

			if (has_post)
			{
				//uint32_t* eax = AllocPreservationData()
				a.call(Imm(AllocPreservationData));

				//preserve original return address
				a.mov(x86::edx, x86::esp);
				a.mov(x86::ecx, x86::dword_ptr_rel(12, x86::edx));
				a.mov(x86::dword_ptr_rel(0, x86::eax), x86::ecx);

				//preserve ebp
				a.mov(x86::dword_ptr_rel(4, x86::eax), x86::ebp);
				a.mov(x86::ebp, x86::eax);
			}

			if (has_pre)
			{
				//this->ExecuteLuaPreFunction();
				a.mov(x86::ecx, Imm(this_ptr));
				a.call(Imm(union_cast<uintptr_t>(&LuaDetour::ExecuteLuaPreFunction)));
			}

			LoadASMRegisters(a);

			if (!has_post)
			{
				a.jmp(x86::dword_ptr_abs(reinterpret_cast<uint64_t>(original_function_ptr)));
				return AddToRuntime(code, runtime);
			}

			a.add(x86::esp, 4); //add 4 back to the stack pointer so that call will use our return address

			a.call(x86::dword_ptr_abs(reinterpret_cast<uint64_t>(original_function_ptr)));

			a.sub(x86::esp, 4); //sub 4 to the stack pointer so we have room to inject a new return address

			SaveASMRegisters(a);
			
			a.mov(x86::ecx, Imm(this_ptr));
			a.call(Imm(union_cast<uintptr_t>(&LuaDetour::ExecuteLuaPostFunction)));

			//restore ebx
			a.mov(x86::eax, x86::ebp);
			a.mov(x86::ebp, x86::dword_ptr_rel(4, x86::eax));

			//restore original return address;
			a.mov(x86::ecx, x86::dword_ptr_rel(0, x86::eax));
			a.mov(x86::edx, x86::esp);
			a.mov(x86::dword_ptr_rel(12, x86::edx), x86::ecx);

			//FreePreservationData(eax)
			a.push(x86::eax);
			a.call(Imm(FreePreservationData));
			a.add(x86::esp, 4);

			LoadASMRegisters(a);

			a.ret();

			return AddToRuntime(code, runtime);
		}
	};
}

static vector<HANDLE> GetAllThreadHandles()
{
	vector<HANDLE> handles;
	handles.reserve(8);

	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD pid = GetCurrentProcessId();
		DWORD tid = GetCurrentThreadId();
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te))
		{
			do {
				if (te.th32OwnerProcessID == pid && te.th32ThreadID != tid) {
					HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, te.th32ThreadID);
					handles.push_back(hThread);
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}

	return handles;
}

static void CloseAllThreadHandles(vector<HANDLE>& handles)
{
	for (const HANDLE handle : handles)
	{
		CloseHandle(handle);
	}
	handles.clear();
}

static void DetourUpdateThreads(vector<HANDLE>& handles)
{
	for (const HANDLE handle : handles)
	{
		DetourUpdateThread(handle);
	}
}

LuaDetour::LuaDetour(LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn)
: mFunctionInfo(function_info)
, mOriginalFunction(mFunctionInfo.GetFunctionPointer())
{

	const bool has_pre = pre_fn.has_value();
	const bool has_post = post_fn.has_value();

	if (has_pre)
		GetLuaSpore().CopyFunctionToAllStates(pre_fn.value(), mLuaPreFunction);

	if (has_post)
		GetLuaSpore().CopyFunctionToAllStates(post_fn.value(), mLuaPostFunction);

	mDetourPreFunction = DetourFunctionRunLuaCallback::Get(mRuntime, has_pre, has_post, this, &mOriginalFunction);

	DetourRestoreAfterWith();
	DetourTransactionBegin();
	auto handles = GetAllThreadHandles();
	DetourUpdateThreads(handles);
	DetourAttach(&mOriginalFunction, mDetourPreFunction);
	DetourTransactionCommit();
	CloseAllThreadHandles(handles);
}

LuaDetour::~LuaDetour()
{
	DetourTransactionBegin();
	auto handles = GetAllThreadHandles();
	DetourUpdateThreads(handles);
	DetourDetach(&mOriginalFunction, mDetourPreFunction);
	DetourTransactionCommit();
	CloseAllThreadHandles(handles);

	mRuntime._release(mDetourPreFunction);
	mDetourPreFunction = nullptr;
}

void LuaDetour::ExecuteLuaPreFunction()
{
	ZoneScoped;
	GetLuaSpore().ExecuteOnFreeState([this](lua_State* L)
	{
		if (const auto* fn = mLuaPreFunction.get(L))
		{
			std::ignore = fn->call();
		}
	});
}

void LuaDetour::ExecuteLuaPostFunction()
{
	ZoneScoped;
	GetLuaSpore().ExecuteOnFreeState([this](lua_State* L)
	{
		if (const auto* fn = mLuaPostFunction.get(L))
		{
			std::ignore = fn->call();
		}
	});
}

static void AddLuaDetour(const LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn)
{
	new LuaDetour(function_info, pre_fn, post_fn);
}

OnLuaInit(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;

	s.new_usertype<LuaDetourFunctionInfo>(
		"DetourFunctionInfo",
		sol::call_constructor, sol::initializers(
			[](LuaDetourFunctionInfo* memory, const uintptr_t disk_address, const uintptr_t address = 0x0)
			{
				auto* self = new(memory) LuaDetourFunctionInfo();
				self->disk_address = disk_address;
				self->address = address != 0x0 ? address : disk_address;
			}
		),
		"disk_address", &LuaDetourFunctionInfo::disk_address,
		"address", &LuaDetourFunctionInfo::address,
		sol::meta_function::to_string, [](const LuaDetourFunctionInfo& info) { return eastl::string().sprintf("LuaDetourFunctionInfo address(0x%08X) disk_address(0x%08X)", info.address, info.disk_address); }
	);

	s["AddDetour"] = AddLuaDetour;
}

#endif