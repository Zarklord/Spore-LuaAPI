/****************************************************************************
* Copyright (C) 2023-2024 Zarklord
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
#include <LuaSpore/DefaultIncludes.h>

class LuaSpore
{
public:
	LUAAPI [[nodiscard]] lua_State* GetLuaState() const;
	//intentionally not LUAAPI, that way sol::state_view is the mod's version, and not the version in the LuaAPI
	[[nodiscard]] sol::state_view GetState() const
	{
		return {GetLuaState()};
	}
	LUAAPI bool DoLuaFile(sol::string_view file) const;
	LUAAPI static sol::optional<sol::function> LoadLuaBuffer(sol::string_view package, sol::string_view group, sol::string_view instance);
	LUAAPI static bool LuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance);
	LUAAPI static eastl::vector<sol::u16string_view> GetPackages();
LUA_INTERNALPUBLIC:
	static void Initialize();
	static void Finalize();
	static LuaSpore& Get();
	static bool Exists();

	bool InternalDoLuaFile(sol::string_view file) const;
	static sol::optional<sol::function> InternalLoadLuaBuffer(sol::string_view package, sol::string_view group, sol::string_view instance);
	static bool InternalLuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance);
	
	void PostInit() const;
public:
	LuaSpore(const LuaSpore&) = delete;
    LuaSpore& operator=(const LuaSpore&) = delete;
	LuaSpore(LuaSpore&&) = delete;
    LuaSpore& operator=(LuaSpore&&) = delete;
private:
	static LuaSpore* mInstance;

	LuaSpore();
	~LuaSpore();
private:
	void Update(double dt) const;

	static void LoadLuaGlobals(sol::state& s);

	void LocateLuaMods();
	Resource::Database* GetDatabase(sol::string_view dbpf_name);
	std::optional<eastl::string16> GetFolder(sol::string_view folder_name) const;
	const eastl::string16& GetLuaDevAbsolute();

	static constexpr const char16_t* luadev_folder = u"luadev";

	sol::state mState;

	sol::function mLuaTraceback;
	sol::function mLuaUpdate;

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