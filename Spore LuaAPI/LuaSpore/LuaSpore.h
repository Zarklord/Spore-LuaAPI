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

#include <LuaSpore/LuaInternal.h>

#include <mutex>

#include "LuaMutex.h"
#include "LuaSporeMultiReference.h"
#include "SporeDetours.h"

LUAAPI void lua_deepcopyx(lua_State* source, lua_State* dest, int arg);
LUAAPI void lua_deepcopy_args(lua_State* source, lua_State* dest, int offset, int num_values);
LUAAPI void lua_deepcopy_upvalues(lua_State* source, int source_function, lua_State* dest, int dest_function);

class LuaSpore : public App::IUnmanagedMessageListener
{
public:
	LUAAPI static bool IsMainThread();

	LUAAPI [[nodiscard]] MutexedLuaState GetMainLuaState();
	LUAAPI [[nodiscard]] MutexedLuaState GetFreeLuaState();
	LUAAPI [[nodiscard]] MutexedLuaStates GetAllLuaStates();

	LUAAPI [[nodiscard]] bool IsMainState(lua_State* L) const;

	template <typename T>
	void CopyFunctionToAllStates(const sol::function& fn, LuaMultiReference<T>& output)
	{
		output.reset();
		auto main_state = GetMainLuaState();
		sol::bytecode fn_bytecode = fn.dump();
		fn.push(main_state.lua_state());
		const int source_function = lua_gettop(main_state.lua_state());
		for (lua_State* L : GetAllLuaStates())
		{
			auto x = static_cast<sol::load_status>(luaL_loadbufferx(L, reinterpret_cast<const char*>(fn_bytecode.data()), fn_bytecode.size(), "", sol::to_string(sol::load_mode::any).c_str()));
			if (x != sol::load_status::ok)
			{
				lua_pop(L, 1);
				continue;
			}
			lua_deepcopy_upvalues(main_state.lua_state(), source_function, L, lua_gettop(L));
			output.set(L, sol::load_result(L, sol::absolute_index(L, -1), 1, 1, x).get<sol::function>());
		}
		fn.pop();
	}

	LUAAPI bool DoLuaFile(sol::string_view file) const;
	LUAAPI static sol::optional<sol::function> LoadLuaBuffer(sol::string_view package, sol::string_view group, sol::string_view instance);
	LUAAPI static bool LuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance);
	LUAAPI static eastl::vector<sol::u16string_view> GetPackages();
	
	LUAAPI void LockThreadState(lua_State* L) const;
	LUAAPI void UnlockThreadState(lua_State* L) const;

	static void RegisterCPPMod(sol::string_view mod, uint32_t version)
	{
		RegisterAPIMod(mod, version);
		LuaAPI::SporeDetours::Attach();
	}
private:
	LUAAPI [[nodiscard]] static size_t GetNumThreadStates() { return LuaSporeConfiguration::NumThreadStates; }
	LUAAPI static void RegisterAPIMod(sol::string_view mod, uint32_t version);
LUA_INTERNALPUBLIC:
	static void Initialize();
	static void Finalize();
	static LuaSpore& Get();
	static bool Exists();

	bool InternalDoLuaFile(sol::string_view file) const;
	static sol::optional<sol::function> InternalLoadLuaBuffer(sol::state_view s, sol::string_view package, sol::string_view group, sol::string_view instance);
	static bool InternalLuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance);

	static vector<pair<sol::string_view, uint32_t>>& GetCPPMods();
	
	void PostInit();

	virtual bool HandleMessage(uint32_t messageID, void* pMessage) override;
public:
	LuaSpore(const LuaSpore&) = delete;
    LuaSpore& operator=(const LuaSpore&) = delete;
	LuaSpore(LuaSpore&&) = delete;
    LuaSpore& operator=(LuaSpore&&) = delete;
private:
	static inline LuaSpore* sInstance = nullptr;
	static inline std::thread::id sMainThreadId;
	static inline vector<pair<sol::string_view, uint32_t>> sCPPMods;

	LuaSpore();
	~LuaSpore() override;

	void InitializeState(sol::state& s, bool is_main_state);

	void Update(double dt);

	static void LoadLuaGlobals(sol::state& s);

	void LocateLuaMods();
	Resource::Database* GetDatabase(sol::string_view dbpf_name);
	std::optional<eastl::string16> GetFolder(sol::string_view folder_name) const;
	const eastl::string16& GetLuaDevAbsolute();

	static constexpr const char16_t* luadev_folder = u"luadev";

	mutable std::recursive_mutex mStateMutex;
	sol::state mState;
	mutable eastl::vector<std::recursive_mutex*> mThreadStatesMutex;
	eastl::vector<sol::state> mThreadStates;

	sol::function mLuaUpdate;

	Clock mUpdateClock;

	eastl::string16 mAbsoluteLuaDevDir;

	struct case_insensitive_hash
	{
		size_t operator()(const string16& x) const
		{
			const char16_t* p = x.c_str();
			unsigned int c, result = 2166136261U;
			while((c = *p++) != 0)
				result = (result * 16777619) ^ static_cast<unsigned int>(eastl::CharToLower(static_cast<char16_t>(c)));
			return (size_t)result;
		}
	};

	struct case_insensitive_equals
	{
		bool operator()(const string16& a, const string16& b) const
		{
			const size_t a_size = a.size();
			if (a_size != b.size()) return false;
			for (size_t i = 0; i < a_size; ++i)
			{
				if (eastl::CharToLower(a[i]) != eastl::CharToLower(b[i]))
					return false;
			}
			return true;
		}
	};
	
	eastl::hash_map<eastl::string16, Resource::Database*, case_insensitive_hash, case_insensitive_equals> mLuaDatabases;
	eastl::hash_set<eastl::string16, case_insensitive_hash, case_insensitive_equals> mLuaFolders;
};

extern LUAAPI LuaSpore& GetLuaSpore();
extern LUAAPI bool LuaSporeExists();