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
#include <LuaSpore/LuaSporeCallbacks.h>

#include <Spore/Resource/cResourceManager.h>

#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"

#include <EASTL/hash_set.h>
#include <algorithm>
#include <cuchar>

void LuaSpore::Initialize()
{
	new LuaSpore();
}

void LuaSpore::Finalize()
{
	delete sInstance;
}

LuaSpore& LuaSpore::Get()
{
	return *sInstance;
}

bool LuaSpore::Exists()
{
	return sInstance != nullptr;
}

LuaSpore& GetLuaSpore()
{
	return LuaSpore::Get();	
}

bool LuaSporeExists()
{
	return LuaSpore::Exists();
}

int LuaPanic(lua_State* L)
{
	ModAPI::Log("LUA: RUN-TIME ERROR %s", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 0;
}

const char* TracyLuaAllocName = "Lua";
void* LuaStateAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	if (nsize == 0)
	{
		TracyFreeN(ptr, TracyLuaAllocName);
		delete[] static_cast<char*>(ptr);
		return nullptr;
	}
	void* new_ptr = new char[nsize];
	TracyAllocN(new_ptr, nsize, TracyLuaAllocName);
	if (ptr)
	{
		memcpy(new_ptr, ptr, std::min(nsize, osize));
		TracyFreeN(ptr, TracyLuaAllocName);
		delete[] static_cast<char*>(ptr);
	}
	return new_ptr;
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase; // NOLINT
static eastl::string16 GetModAPIRoot()
{
	TCHAR DllPath[MAX_PATH] = {};
	GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), DllPath, _countof(DllPath));

	eastl::string16 lua_dev_folder;
	lua_dev_folder.reserve(MAX_PATH);
	lua_dev_folder.sprintf(u"%ls", DllPath);
	
	//removes the mlibs\\file.dll from the path
	lua_dev_folder.resize(lua_dev_folder.find_last_of('\\')); 
	lua_dev_folder.resize(lua_dev_folder.find_last_of('\\')+1);

	return lua_dev_folder;
}

sol::object LuaSearcher(sol::this_state L, const sol::string_view modulename)
{
	sol::state_view s(L);
	const auto package_end = modulename.find_first_of(".\\/");
	const auto group_end = modulename.find_last_of(".\\/");

	if (package_end == sol::string_view::npos)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is missing a package and group", modulename.data());
		return sol::make_object(s, errmsg);
	}
	else if (group_end == package_end)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is missing a package", modulename.data());
		return sol::make_object(s, errmsg);
	}

	const sol::string_view package = modulename.substr(0, package_end);
	const sol::string_view group = modulename.substr(package_end+1, group_end - (package_end+1));
	const sol::string_view instance = modulename.substr(group_end+1);

	sol::optional<sol::function> fn = LuaSpore::InternalLoadLuaBuffer(s, package, group, instance);
	if (!fn)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is not found", modulename.data());
		return sol::make_object(s, errmsg);
	}
	return sol::make_object(s, fn);
}

static bool SporeLuaExists(const sol::string_view modulename)
{
	const auto package_end = modulename.find_first_of(".\\/");
	const auto group_end = modulename.find_last_of(".\\/");

	if (package_end == sol::string_view::npos || group_end == package_end)
		return false;

	const sol::string_view package = modulename.substr(0, package_end);
	const sol::string_view group = modulename.substr(package_end+1, group_end - (package_end+1));
	const sol::string_view instance = modulename.substr(group_end+1);

	return LuaSpore::InternalLuaFileExists(package, group, instance);
}

static sol::optional<sol::function> SporeLoadLua(sol::this_state L, const sol::string_view modulename)
{
	const auto package_end = modulename.find_first_of(".\\/");
	const auto group_end = modulename.find_last_of(".\\/");

	if (package_end == sol::string_view::npos || group_end == package_end)
		return {};

	const sol::string_view package = modulename.substr(0, package_end);
	const sol::string_view group = modulename.substr(package_end+1, group_end - (package_end+1));
	const sol::string_view instance = modulename.substr(group_end+1);

	return LuaSpore::InternalLoadLuaBuffer(L, package, group, instance);
}

static auto SporeGetPackages()
{
	return sol::as_table(LuaSpore::GetPackages());
}

static auto SporeGetCPPMods()
{
	return sol::as_table(LuaSpore::GetCPPMods());
}

static void SporeRequireOnAllThreads(const sol::string_view module)
{
	for (sol::state_view s : GetLuaSpore().GetAllLuaStates())
	{
		s["require"](module);
	}
}

void lua_deepcopyx(lua_State* source, lua_State* dest, int arg)
{
	switch (static_cast<sol::type>(lua_type(source, arg)))
	{
	case sol::type::string:
	{
		size_t cstring_length;
		const char* cstring = luaL_checklstring(source, arg, &cstring_length);
		lua_pushlstring(dest, cstring, cstring_length);
		break;
	}
	case sol::type::number:
	{
		if (lua_isinteger(source, arg))
		{
			lua_pushinteger(dest, luaL_checkinteger(source, arg));
		}
		else
		{
			lua_pushnumber(dest, luaL_checknumber(source, arg));
		}
		break;
	}
	case sol::type::boolean:
	{
		lua_pushboolean(dest, lua_toboolean(source, arg));
		break;
	}
	case sol::type::table:
	{
		const int abs_arg = lua_absindex(source, arg);
		lua_newtable(dest);
		const int table_idx = lua_absindex(dest, -1);

		lua_pushnil(source);
		while (lua_next(source, abs_arg) != 0)
		{
			const int key_idx = lua_absindex(source, -2);
			const int value_idx = lua_absindex(source, -1);

			lua_deepcopyx(source, dest, key_idx); //copy the key
			const int dest_key_idx = lua_absindex(dest, -1);
			if (static_cast<sol::type>(lua_type(dest, dest_key_idx)) == sol::type::nil)
			{
				lua_pop(dest, 1);
				lua_pop(source, 1);
				continue;
			}
			lua_deepcopyx(source, dest, value_idx); //copy the value
			lua_rawset(dest, table_idx); //set the dest table
			
			lua_pop(source, 1);
		}
		break;
	}
	case sol::type::thread:
	case sol::type::function:
	case sol::type::userdata:
	case sol::type::lightuserdata:
	case sol::type::poly:
	case sol::type::none:
	case sol::type::nil:
		lua_pushnil(dest);
		break;
	}
}

void lua_deepcopy_args(lua_State* source, lua_State* dest, int offset, int num_values)
{
	for (int i = 1; i <= num_values; ++i)
	{
		lua_deepcopyx(source, dest, i + offset);
	}
}

void lua_deepcopy_upvalues(lua_State* source, int source_function, lua_State* dest, int dest_function)
{
	for (int i = 1; i <= 256; ++i)
	{
		const char* source_name = lua_getupvalue(source, source_function, i);
		if (source_name == nullptr) break;

		if (strcmp(source_name, "_ENV") == 0)
		{
			lua_pop(source, 1);
			lua_pushglobaltable(dest);
		}
		else
		{		
			lua_deepcopyx(source, dest, lua_absindex(source, -1));
			lua_pop(source, 1);
		}
		const char* dest_name = lua_setupvalue(dest, dest_function, i);
		assert(dest_name != nullptr && strcmp(source_name, dest_name) == 0);
	}
}

static void SporeExecuteOnAllThreads(sol::this_state main_state, const sol::function& fn, sol::variadic_args va)
{
	sol::bytecode fn_bytecode = fn.dump();

	for (sol::state_view s : GetLuaSpore().GetAllLuaStates())
	{
		lua_getglobal(s, sol::detail::default_handler_name());
		const int error_handler = lua_gettop(s);

		auto x = static_cast<sol::load_status>(luaL_loadbufferx(s, reinterpret_cast<const char*>(fn_bytecode.data()), fn_bytecode.size(), "", sol::to_string(sol::load_mode::any).c_str()));
		if (x != sol::load_status::ok)
		{
			lua_pop(s, 2);
			continue;
		}
		lua_deepcopy_upvalues(main_state, 1, s, lua_gettop(s));
		lua_deepcopy_args(main_state, s, 1, static_cast<int>(va.size()));
		lua_pcall(s, va.size(), 0, error_handler);
		lua_remove(s, error_handler);
	}
}

LuaSpore::LuaSpore()
: mState(LuaPanic, LuaStateAlloc, this)
, mUpdateClock(Clock::Mode::Milliseconds)
{
	sMainThreadId = std::this_thread::get_id();
	sInstance = this;
	mAbsoluteLuaDevDir = GetModAPIRoot();
	mAbsoluteLuaDevDir.append_sprintf(u"\\%ls", luadev_folder);
		
	mState.change_gc_mode_incremental(100, 100, 10);
	mState.stop_gc();

	mThreadStatesMutex.reserve(LuaSporeConfiguration::NumThreadStates);
	mThreadStates.reserve(LuaSporeConfiguration::NumThreadStates);
	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		mThreadStatesMutex.push_back(new std::recursive_mutex);
		mThreadStates.emplace_back(LuaPanic, LuaStateAlloc, this);
	}

	LocateLuaMods();

	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		std::lock_guard lock(*mThreadStatesMutex[i]);
		InitializeState(mThreadStates[i], false);
	}

	InitializeState(mState, true);

	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		std::lock_guard lock(*mThreadStatesMutex[i]);
		LuaAPI::LuaPostInitializers::RunCallbacks(mThreadStates[i], false);
	}
	LuaAPI::LuaPostInitializers::RunCallbacks(mState, true);
}

static int protected_function_error_handler(lua_State* L)
{
	sol::state_view s(L);
	eastl::string message = "An unknown error occured";
	
	sol::optional<sol::string_view> top_message = sol::stack::unqualified_check_get<sol::string_view>(s, 1, &sol::no_panic);
	if (top_message)
		message.assign(top_message->data(), top_message->size());

	bool first_error = s["_TRACEBACK_FIRST_ERROR"];
	if (!first_error)
	{
		s["_TRACEBACK_FIRST_ERROR"] = true;
		
		auto traceback_handler = s["_TRACEBACK"];
		if (traceback_handler.get_type() == sol::type::function)
		{
			sol::optional<sol::string_view> traceback_message = traceback_handler(sol::string_view(message.data(), message.size()));
			if (traceback_message)
				message.assign(traceback_message->data(), traceback_message->size());
		}

		ModAPI::Log(message.c_str());
	}
	else
	{
		bool second_error = s["_TRACEBACK_SECOND_ERROR"];
		if (!second_error)
		{
			s["_TRACEBACK_SECOND_ERROR"] = true;
			ModAPI::Log("Script error after script error was thrown: %s", message.c_str());
		}
	}

	return sol::stack::push(s, message);
}

static void ResetLuaSporeErrors(sol::this_state L)
{
	sol::state_view s(L);
	s["_TRACEBACK_FIRST_ERROR"] = false;
	s["_TRACEBACK_SECOND_ERROR"] = false;
};

void LuaSpore::InitializeState(sol::state& s, bool is_main_state)
{
	ZoneScoped;
	s.open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::coroutine,
		sol::lib::string,
		sol::lib::os,
		sol::lib::math,
		sol::lib::table,
		sol::lib::debug,
		sol::lib::io,
		sol::lib::utf8
	);
	
	s.script("_TRACEBACK = debug.traceback");

	s["ResetLuaSporeErrors"] = ResetLuaSporeErrors;
	ResetLuaSporeErrors(s.lua_state());

	sol::protected_function::set_default_handler(sol::object(s, sol::in_place, protected_function_error_handler));

	s["MAIN_STATE"] = is_main_state;
	
	s["SporeLuaExists"] = SporeLuaExists;
	s["SporeLoadLua"] = SporeLoadLua;
	s["GetSporeDBPFNames"] = SporeGetPackages;
	s["GetCPPMods"] = SporeGetCPPMods;

	if (is_main_state)
	{
		s["RequireOnAllThreads"] = SporeRequireOnAllThreads;
		s["ExecuteOnAllThreads"] = SporeExecuteOnAllThreads;
	}

	tracy::LuaRegister(s);

	auto package_searchers = s["package"]["searchers"];
	if (package_searchers.valid())
	{
		package_searchers[2] = LuaSearcher;
		package_searchers[3] = sol::nil;
		package_searchers[4] = sol::nil;
	}

	LoadLuaGlobals(s);

	LuaAPI::LuaInitializers::RunCallbacks(s, is_main_state);

	if (is_main_state)
	{
		if (!InternalDoLuaFile("luaspore/scripts/main"))
		{
			return;
		}

		mLuaUpdate = s["Update"];
	}
}

LuaSpore::~LuaSpore()
{
	MessageManager.RemoveListener(this, App::kMsgAppUpdate);

	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		std::lock_guard lock(*mThreadStatesMutex[i]);
		LuaAPI::LuaDisposers::RunCallbacks(mThreadStates[i], false);
	}
	LuaAPI::LuaDisposers::RunCallbacks(mState, true);
	
	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		delete mThreadStatesMutex[i];
	}
	sInstance = nullptr;
}

void LuaSpore::PostInit()
{
	MessageManager.AddUnmanagedListener(this, App::kMsgAppUpdate);
	mUpdateClock.Reset();
	mUpdateClock.Start();
}

bool LuaSpore::HandleMessage(uint32_t messageID, void* pMessage)
{
	const double dt = mUpdateClock.GetElapsedTime() / 1000.0;
	mUpdateClock.Reset();
	mUpdateClock.Start();
	Update(dt);

	return false;
}

void LuaSpore::Update(double dt)
{
	if (mLuaUpdate)
	{
		mLuaUpdate.call(dt);
		const auto result = mLuaUpdate(dt);
	}

	ZoneScopedN("LuaGC");
	mState.step_gc(0);
}

MutexedLuaState LuaSpore::GetMainLuaState()
{
	return {&mStateMutex, mState};
}

MutexedLuaState LuaSpore::GetFreeLuaState()
{
	if (IsMainThread())
	{
		return GetMainLuaState();
	}
	
	while (true)
	{
		for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
		{
			auto* mutex = mThreadStatesMutex[i];
			if (mutex->try_lock())
			{
				return {mutex, mThreadStates[i].lua_state(), MutexedLuaState::lock_already_acquired{}};
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

MutexedLuaStates LuaSpore::GetAllLuaStates()
{
	MutexedLuaStates mutexed_lua_states;
	//main state always goes last
	mutexed_lua_states.SetState(LuaSporeConfiguration::NumThreadStates, GetMainLuaState());
	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		mutexed_lua_states.SetState(i, MutexedLuaState{mThreadStatesMutex[i], mThreadStates[i].lua_state()});
	}
	return mutexed_lua_states;
}

bool LuaSpore::IsMainState(lua_State* L) const
{
	return mState.lua_state() == L;
}

void LuaSpore::LockThreadState(lua_State* L) const
{
	if (L == mState)
	{
		mStateMutex.lock();
		return;
	}

	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		if (L == mThreadStates[i])
		{
			mThreadStatesMutex[i]->lock();
			return;
		}
	}

	assert(false);
}

void LuaSpore::UnlockThreadState(lua_State* L) const
{
	if (L == mState)
	{
		mStateMutex.unlock();
		return;
	}

	for (size_t i = 0; i < LuaSporeConfiguration::NumThreadStates; ++i)
	{
		if (L == mThreadStates[i])
		{
			mThreadStatesMutex[i]->unlock();
			return;
		}
	}

	assert(false);
}

void LuaSpore::RegisterAPIMod(sol::string_view mod, uint32_t version)
{
	sCPPMods.push_back({mod, version});
}

bool LuaSpore::DoLuaFile(const sol::string_view file) const
{
	return InternalDoLuaFile(file);
}

bool LuaSpore::InternalDoLuaFile(sol::string_view file) const
{
	std::lock_guard lock(mStateMutex);

	ModAPI::Log("DoLuaFile %.*s.lua", static_cast<int>(file.length()), file.data());
	
	const auto& result = mState["require"](file);
	if (!result.valid())
	{
		const sol::error err = result;
		ModAPI::Log("DoLuaFile Error running lua file %.*s.lua:\n%s", static_cast<int>(file.length()), file.data(), err.what());
		return false;
	}
	return true;
}

sol::optional<sol::function> LuaSpore::LoadLuaBuffer(sol::string_view package, sol::string_view group, sol::string_view instance)
{
	auto main_state = GetLuaSpore().GetMainLuaState();
	return InternalLoadLuaBuffer(main_state.lua_state(), package, group, instance);
}

sol::optional<sol::function> LuaSpore::InternalLoadLuaBuffer(sol::state_view s, sol::string_view package, sol::string_view group, sol::string_view instance)
{
	LuaSpore& lua_spore = GetLuaSpore();
	eastl::vector<char*> data;

	if (const auto folder = lua_spore.GetFolder(package))
	{
		eastl::string16 fullpath;
		fullpath.sprintf(u"%ls\\%s\\%.*hs\\%.*hs.lua", lua_spore.GetLuaDevAbsolute().c_str(),
			folder->c_str(),
			static_cast<int>(group.length()), group.data(),
			static_cast<int>(instance.length()), instance.data()
		);

		if (!IO::File::Exists(fullpath.c_str()))
		{
			return {};
		}

		const FileStreamPtr lua_file = new IO::FileStream(fullpath.c_str());
		if (!lua_file->Open(IO::AccessFlags::Read, IO::CD::OpenExisting))
		{
			return {};
		}

		const auto size = lua_file->GetSize();
		data.resize(size);
		lua_file->Read(data.data(), size);
		lua_file->Close();
	}
	else
	{
		const ResourceKey lua_file(id(instance), id("lua"), id(group));

		Resource::IRecord* record;
		const auto database = lua_spore.GetDatabase(package);
		if (!database || !database->OpenRecord(lua_file, &record))
		{
			return {};
		}

		const auto size = record->GetStream()->GetSize();
		data.resize(size);
		record->GetStream()->Read(data.data(), size);
		record->RecordClose();
	}

	eastl::string filename;
	filename.sprintf("%.*s/%.*s/%.*s.lua", 
		static_cast<int>(package.length()), package.data(),
		static_cast<int>(group.length()), group.data(),
		static_cast<int>(instance.length()), instance.data()
	);

	return s.load_buffer(reinterpret_cast<const char*>(data.data()), data.size(), filename.c_str(), sol::load_mode::text);
}

bool LuaSpore::LuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance)
{
	return InternalLuaFileExists(package, group, instance);
}

bool LuaSpore::InternalLuaFileExists(sol::string_view package, sol::string_view group, sol::string_view instance)
{
	LuaSpore& lua_spore = GetLuaSpore();
	eastl::vector<char*> data;

	if (const auto folder = lua_spore.GetFolder(package))
	{
		eastl::string16 fullpath;
		fullpath.sprintf(u"%ls\\%s\\%.*hs\\%.*hs.lua", lua_spore.GetLuaDevAbsolute().c_str(),
			folder->c_str(),
			static_cast<int>(group.length()), group.data(),
			static_cast<int>(instance.length()), instance.data()
		);

		return IO::File::Exists(fullpath.c_str());
	}
	else
	{
		const ResourceKey lua_file(id(instance), id("lua"), id(group));

		Resource::IRecord* record;
		const auto database = lua_spore.GetDatabase(package);
		if (!database || !database->OpenRecord(lua_file, &record))
		{
			return false;
		}
		record->RecordClose();
		return true;
	}
}

vector<sol::u16string_view> LuaSpore::GetPackages()
{
	LuaSpore& lua_spore = GetLuaSpore();
	vector<sol::u16string_view> packages;

	for (auto& package : lua_spore.mLuaFolders)
	{
		packages.push_back({package.c_str(), package.length()});
	}

	for (auto& [package, database] : lua_spore.mLuaDatabases)
	{
		if (lua_spore.mLuaFolders.count(package) == 1) continue;
		packages.push_back({package.c_str(), package.length()});
	}

	return packages;
}

vector<pair<sol::string_view, uint32_t>>& LuaSpore::GetCPPMods()
{
	return sCPPMods;
}

bool LuaSpore::IsMainThread()
{
	return sMainThreadId == std::this_thread::get_id();
}

static eastl::string16 GetPackageNameFromDatabase(const Resource::Database* database)
{
	const eastl::string16 location = database->GetLocation();
	const auto location_length = location.size();
	if (location.find(u".package",  location_length - 8) != eastl::string16::npos)
	{
		const auto last_slash = location.find_last_of(u"/\\") + 1;
		return location.substr(last_slash, location_length - last_slash - 8);
	}
	return u"";
}

void LuaSpore::LocateLuaMods()
{
	ResourceKey base_package{id("main"), id("lua"), id("scripts")};
	ResourceKey mods{id("main"), id("lua"), id("mod")};

	Resource::IResourceManager::DatabaseList databases;
	ResourceManager.GetDatabaseList(databases, &base_package);
	ResourceManager.GetDatabaseList(databases, &mods);
	for (const auto database : databases)
	{
		auto package_name = GetPackageNameFromDatabase(database);
		if (!package_name.empty())
		{
			mLuaDatabases[package_name] = database;
		}
	}

	eastl::string16 luadev_dir;
	luadev_dir.append_sprintf(u"%ls\\*", mAbsoluteLuaDevDir.c_str());

	const eastl::wstring find_dir = reinterpret_cast<const wchar_t*>(luadev_dir.c_str());
	
	WIN32_FIND_DATA FileData;
	const HANDLE hFind = FindFirstFileW(find_dir.c_str(), &FileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
		    if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		    {
				//skip "." and ".." directories
				if (FileData.cFileName[0] == '.')
				{
					if (FileData.cFileName[1] == 0 || (FileData.cFileName[1] == '.' && FileData.cFileName[2] == 0))
						continue;
				}

				eastl::string16 folder_name = reinterpret_cast<char16_t*>(FileData.cFileName);
				mLuaFolders.insert(folder_name);
		    }
		} while (FindNextFileW(hFind, &FileData) != 0);

		FindClose(hFind);
	}
}

Resource::Database* LuaSpore::GetDatabase(sol::string_view dbpf_name)
{
	eastl::string16 dbpf_name_16;
	dbpf_name_16.sprintf(u"%.*hs", static_cast<int>(dbpf_name.length()), dbpf_name.data());
	const auto it = mLuaDatabases.find(dbpf_name_16);
	return it != mLuaDatabases.end() ? it->second : nullptr;
}

std::optional<eastl::string16> LuaSpore::GetFolder(sol::string_view folder_name) const
{
	eastl::string16 folder_name_16;
	folder_name_16.sprintf(u"%.*hs", static_cast<int>(folder_name.length()), folder_name.data());
	const auto it = mLuaFolders.find(folder_name_16);
	return it != mLuaFolders.end() ? *it : std::optional<eastl::string16>();
}

const eastl::string16& LuaSpore::GetLuaDevAbsolute()
{
	return mAbsoluteLuaDevDir;
}

#endif