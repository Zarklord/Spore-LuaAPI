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

#include <asmjit/asmjit.h>

struct LuaDetourFunctionInfo
{
	uintptr_t disk_address{};
	uintptr_t address{};

	void* GetFunctionPointer() const
	{
		return reinterpret_cast<void*>(Address(ModAPI::ChooseAddress(disk_address, address))); //NOLINT
	}
};

class ILuaDetour
{
	friend class LuaDetourDestroyer;
public:
	ILuaDetour() = default;
	ILuaDetour(const ILuaDetour&) = delete;
	ILuaDetour& operator=(const ILuaDetour&) = delete;
	ILuaDetour(ILuaDetour&&) = delete;
	ILuaDetour& operator=(ILuaDetour&&) = delete;
	
	bool IsDetourAttached() const { return mAttached; }

	virtual void AttachDetour() = 0;
	virtual void DetachDetour() = 0;

	void QueueDestroy();
protected:
	void DoAttachDetour(void** original_function, void* detour_function);
	void DoDetachDetour(void** original_function, void* detour_function);

	virtual bool IsSafeToDelete() const = 0;
	virtual ~ILuaDetour() = default;
private:

	bool mAttached = false;
};

class LuaDetourDestroyer final : public App::IUnmanagedMessageListener
{
	friend class ILuaDetour;
public:
	LuaDetourDestroyer() = default;
	~LuaDetourDestroyer() override;
	LuaDetourDestroyer(const LuaDetourDestroyer&) = delete;
	LuaDetourDestroyer& operator=(const LuaDetourDestroyer&) = delete;
	LuaDetourDestroyer(LuaDetourDestroyer&&) = delete;
	LuaDetourDestroyer& operator=(LuaDetourDestroyer&&) = delete;
private:
	void AddDetourToDelete(ILuaDetour* detour);

	void StartUpdating();
	void StopUpdating();
	
	virtual bool HandleMessage(uint32_t messageID, void* pMessage) override;

	bool mUpdating = false;
	fixed_vector<ILuaDetour*, 4> mDetoursToDelete;
};

class UnknownFunctionDetour : public ILuaDetour
{
public:
	UnknownFunctionDetour(LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn);

	void AttachDetour() override;
	void DetachDetour() override;

	void ExecuteLuaPreFunction();
	void ExecuteLuaPostFunction();

	void* AllocCallData();
	void FreeCallData(const uint32_t* data);
private:
	int GetNumActiveCalls() const { return mActiveCallInstances.load(std::memory_order_relaxed); }
	bool IsSafeToDelete() const override { return GetNumActiveCalls() == 0; }
	~UnknownFunctionDetour() override;

	LuaDetourFunctionInfo mFunctionInfo{};
	
	LuaMultiReference<sol::function> mLuaPreFunction{};
	LuaMultiReference<sol::function> mLuaPostFunction{};

	asmjit::JitRuntime mRuntime;

	void* mOriginalFunction = nullptr;
	void* mDetourFunction = nullptr;

	std::atomic<int> mActiveCallInstances{0};
};

#endif