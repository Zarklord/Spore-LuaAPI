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
#include <SourceCode/LuaDetours/LuaDetour.h>

class LuaDetourBinding
{
public:
	LuaDetourBinding(LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn)
	: mDetour(new UnknownFunctionDetour(function_info, pre_fn, post_fn))
	{
		AttachDetour();
	}

	~LuaDetourBinding()
	{
		//assert(!mSelfReference.valid());

		mDetour->QueueDestroy();
		mDetour = nullptr;
	}

	LuaDetourBinding(const LuaDetourBinding&) = delete;
	LuaDetourBinding& operator=(const LuaDetourBinding&) = delete;
	LuaDetourBinding(LuaDetourBinding&&) = delete;
	LuaDetourBinding& operator=(LuaDetourBinding&&) = delete;

	void AttachDetour() const
	{
		mDetour->AttachDetour();
	}

	void DetachDetour() const
	{
		mDetour->DetachDetour();
	}

	bool IsDetourAttached() const
	{
		return mDetour->IsDetourAttached();
	}

	void SetReference(const sol::stack_reference& self_reference)
	{
		mSelfReference = sol::reference(self_reference);
	}

	void Delete()
	{
		mSelfReference.reset();
	}
private:

	sol::reference mSelfReference;
	ILuaDetour* mDetour;
};

namespace
{
	auto CreateUnknownFunctionDetour(sol::this_state L, const LuaDetourFunctionInfo function_info, const sol::optional<sol::function>& pre_fn, const sol::optional<sol::function>& post_fn)
	{
		sol::stack_object detour_reference = sol::make_reference_userdata<LuaDetourBinding, sol::stack_object>(L, function_info, pre_fn, post_fn);
		detour_reference.as<LuaDetourBinding>().SetReference(detour_reference);
		return detour_reference;
	}
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

	s.new_usertype<LuaDetourBinding>(
		"LuaDetour",
		sol::no_constructor,
		"Attach", &LuaDetourBinding::AttachDetour,
		"Detach", &LuaDetourBinding::DetachDetour,
		"IsAttached", &LuaDetourBinding::IsDetourAttached,
		"Delete", &LuaDetourBinding::Delete,
		sol::meta_function::to_string, [](const LuaDetourBinding& lua_detour)
		{
			return string().sprintf("LuaDetourBinding (%p)", &lua_detour);
		}
	);

	s["CreateDetour"] = CreateUnknownFunctionDetour;
}

#endif