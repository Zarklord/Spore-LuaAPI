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
#include <LuaSpore/TracyUtil.h>

#include <SourceCode/LuaDetours/LuaDetour.h>
#include <SourceCode/LuaDetours/DetourHelper.h>

#include <asmjit/asmjit.h>
#include <detours.h>

LuaDetourDestroyer::~LuaDetourDestroyer()
{
	StopUpdating();
	for (const ILuaDetour* detour : mDetoursToDelete)
	{
		delete detour;
	}
}

void LuaDetourDestroyer::StartUpdating()
{
	if (mUpdating) return;
	mUpdating = true;
	MessageManager.AddUnmanagedListener(this, App::kMsgAppUpdate);
}

void LuaDetourDestroyer::StopUpdating()
{
	if (mUpdating) return;
	mUpdating = false;
	MessageManager.RemoveListener(this, App::kMsgAppUpdate);
}

bool LuaDetourDestroyer::HandleMessage(const uint32_t messageID, void* pMessage)
{
	assert(messageID == App::kMsgAppUpdate);

	bool deleted_detour = false;
	for (ILuaDetour*& detour : mDetoursToDelete)
	{
		if (detour->IsSafeToDelete())
		{
			delete detour;
			detour = nullptr;
			deleted_detour = true;
		}
	}

	if (deleted_detour)
	{
		const auto it_begin = mDetoursToDelete.begin();
		const auto it_end = mDetoursToDelete.end();
		mDetoursToDelete.erase(remove_if(it_begin, it_end, [](const ILuaDetour* detour){ return detour == nullptr; }), it_end);

		if (mDetoursToDelete.empty())
			StopUpdating();
	}

	return false;
}

void LuaDetourDestroyer::AddDetourToDelete(ILuaDetour* detour)
{
	mDetoursToDelete.push_back(detour);
	StartUpdating();
}

static LuaDetourDestroyer sLuaDetourDestroyer;

using namespace asmjit;

void ILuaDetour::DoAttachDetour(void** original_function, void* detour_function)
{
	if (mAttached) return;
	ZoneScoped;
	
	DetourTransactionBegin();
	detour_thread_manager.SetDetourUpdateThreads();
	DetourAttach(original_function, detour_function);
	DetourTransactionCommit();
	detour_thread_manager.ClearDetourUpdateThreads();

	mAttached = true;
}

void ILuaDetour::DoDetachDetour(void** original_function, void* detour_function)
{
	if (!mAttached) return;
	ZoneScoped;
	
	DetourTransactionBegin();
	detour_thread_manager.SetDetourUpdateThreads();
	DetourDetach(original_function, detour_function);
	DetourTransactionCommit();
	detour_thread_manager.ClearDetourUpdateThreads();

	mAttached = false;
}

void ILuaDetour::QueueDestroy()
{
	DetachDetour();

	sLuaDetourDestroyer.AddDetourToDelete(this);
}

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
	
	namespace DetourFunctionRunLuaCallback
	{
		const void* UnknownFunctionDetour_FreeCallData = union_cast<void*>(&UnknownFunctionDetour::FreeCallData);
		__declspec(naked) void __cdecl ExitDetour()
		{
			__asm
			{
				call UnknownFunctionDetour_FreeCallData
				
				pop edx
				pop ecx
				pop eax

				ret
			}
		}
		void* Get(JitRuntime& runtime, bool has_pre, bool has_post, void* this_ptr, void** original_function_ptr)
		{
			thread_local CodeHolder code;
			code.init(runtime.environment(), runtime.cpuFeatures());
			//code.setLogger(&sLuaSporeASMJitLogger);
			x86::Assembler a(&code);

			SaveASMRegisters(a);

			if (has_post)
			{
				//uint32_t* eax = this->AllocCallData()
				a.mov(x86::ecx, Imm(this_ptr));
				a.call(Imm(union_cast<uintptr_t>(&UnknownFunctionDetour::AllocCallData)));

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
				a.call(Imm(union_cast<uintptr_t>(&UnknownFunctionDetour::ExecuteLuaPreFunction)));
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
			a.call(Imm(union_cast<uintptr_t>(&UnknownFunctionDetour::ExecuteLuaPostFunction)));

			//restore ebx
			a.mov(x86::eax, x86::ebp);
			a.mov(x86::ebp, x86::dword_ptr_rel(4, x86::eax));

			//restore original return address;
			a.mov(x86::ecx, x86::dword_ptr_rel(0, x86::eax));
			a.mov(x86::edx, x86::esp);
			a.mov(x86::dword_ptr_rel(12, x86::edx), x86::ecx);

			//ExitDetour(ecx, eax)
			a.push(x86::eax);
			a.mov(x86::ecx, Imm(this_ptr));
			a.jmp(Imm(ExitDetour));
			//a.call(Imm(FreePreservationData));
			//a.add(x86::esp, 4);

			//LoadASMRegisters(a);

			//a.ret();

			return AddToRuntime(code, runtime);
		}
	};
}


UnknownFunctionDetour::UnknownFunctionDetour(LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn)
: mFunctionInfo(function_info)
, mOriginalFunction(mFunctionInfo.GetFunctionPointer())
{

	const bool has_pre = pre_fn.has_value();
	const bool has_post = post_fn.has_value();

	if (has_pre)
		GetLuaSpore().CopyFunctionToAllStates(pre_fn.value(), mLuaPreFunction);

	if (has_post)
		GetLuaSpore().CopyFunctionToAllStates(post_fn.value(), mLuaPostFunction);

	mDetourFunction = DetourFunctionRunLuaCallback::Get(mRuntime, has_pre, has_post, this, &mOriginalFunction);
}

UnknownFunctionDetour::~UnknownFunctionDetour()
{
	assert(!IsDetourAttached());
	assert(GetNumActiveCalls() == 0);

	mRuntime._release(mDetourFunction);
	mDetourFunction = nullptr;
}

void UnknownFunctionDetour::AttachDetour()
{
	DoAttachDetour(&mOriginalFunction, mDetourFunction);
}

void UnknownFunctionDetour::DetachDetour()
{
	DoDetachDetour(&mOriginalFunction, mDetourFunction);
}

void UnknownFunctionDetour::ExecuteLuaPreFunction()
{
	if (!IsDetourAttached()) return;

	ZoneScoped;

	if (!mLuaPostFunction)
	{
		mActiveCallInstances.fetch_add(1, std::memory_order_relaxed);
	}

	auto free_state = GetLuaSpore().GetFreeLuaState();
	if (const auto* fn = mLuaPreFunction.get(free_state))
	{
		std::ignore = fn->call();
	}

	if (!mLuaPostFunction)
	{
		mActiveCallInstances.fetch_sub(1, std::memory_order_relaxed);
	}
}

void UnknownFunctionDetour::ExecuteLuaPostFunction()
{
	if (!IsDetourAttached()) return;

	ZoneScoped;
	auto free_state = GetLuaSpore().GetFreeLuaState();
	if (const auto* fn = mLuaPostFunction.get(free_state))
	{
		std::ignore = fn->call();
	}
}

void* UnknownFunctionDetour::AllocCallData()
{
	if (TracyIsStarted)
	{
		ZonePushN("UnkownFunctionDetour");
	}
	mActiveCallInstances.fetch_add(1, std::memory_order_relaxed);
	return new uint32_t[2];
}

void UnknownFunctionDetour::FreeCallData(const uint32_t* data)
{
	if (TracyIsStarted)
	{
		ZonePop;
	}
	delete[] data;
	mActiveCallInstances.fetch_sub(1, std::memory_order_relaxed);
}

#endif