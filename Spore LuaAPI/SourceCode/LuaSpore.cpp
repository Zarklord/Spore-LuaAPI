#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <LuaSpore/LuaBinding.h>
#include <LuaSpore/LuaAPI.h>

#include <Spore/Resource/cResourceManager.h>

#include <EASTL/hash_set.h>
#include <algorithm>
#include <cuchar>

namespace LuaAPI
{
	constexpr int MAX_MODS = 2048;
	eastl::fixed_vector<LuaFunction, MAX_MODS> sLuaInitFunctions;
	eastl::fixed_vector<LuaFunction, MAX_MODS> sLuaPostInitFunctions;
	eastl::fixed_vector<LuaFunction, MAX_MODS> sLuaDisposeFunctions;

	void AddLuaInitFunction(LuaFunction f)
	{
		sLuaInitFunctions.push_back(f);
	}
	void AddLuaPostInitFunction(LuaFunction f)
	{
		sLuaPostInitFunctions.push_back(f);
	}
	void AddLuaDisposeFunction(LuaFunction f)
	{
		sLuaDisposeFunctions.push_back(f);
	}
}

LuaSpore* LuaSpore::mInstance = nullptr;
void LuaSpore::Initialize()
{
	new LuaSpore();
}

void LuaSpore::Finalize()
{
	delete mInstance;
}

LuaSpore& LuaSpore::Get()
{
	return *mInstance;
}

bool LuaSpore::Exists()
{
	return mInstance != nullptr;
}

LuaSpore& GetLuaSpore()
{
	return LuaSpore::Get();	
}

int LuaPanic(lua_State* L)
{
	ModAPI::Log("LUA: RUN-TIME ERROR %s", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 0;
}

void* LuaStateAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	if (nsize == 0)
	{
		delete[] static_cast<char*>(ptr);
		return nullptr;
	}

	const auto result = new char[nsize];
	if (ptr)
	{
		memcpy(result, ptr, std::min(nsize, osize));
		delete[] static_cast<char*>(ptr);
	}
	return result;
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
static eastl::string16 GetModAPIRoot()
{
	TCHAR DllPath[MAX_PATH] = {0};
	GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), DllPath, _countof(DllPath));

	eastl::string16 lua_dev_folder;
	lua_dev_folder.reserve(MAX_PATH);
	lua_dev_folder.sprintf(u"%ls", DllPath);
	
	//removes the mlibs\\file.dll from the path
	lua_dev_folder.resize(lua_dev_folder.find_last_of('\\')); 
	lua_dev_folder.resize(lua_dev_folder.find_last_of('\\')+1);

	return lua_dev_folder;
}

sol::object LuaSearcher(sol::this_state L, const sol::string_view& modulename)
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

	sol::optional<sol::function> fn = LuaSpore::LoadLuaBuffer(package.data(), group.data(), instance.data());
	if (!fn)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is not found", modulename.data());
		return sol::make_object(s, errmsg);
	}
	return sol::make_object(s, fn);
}

LuaSpore::LuaSpore()
: mState(LuaPanic, LuaStateAlloc, this)
{
	mInstance = this;
	mAbsoluteLuaDevDir = GetModAPIRoot();
	mAbsoluteLuaDevDir.append_sprintf(u"\\%ls", luadev_folder);

	LocateLuaMods();
	
	mState.open_libraries(
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

	auto package_searchers = mState["package"]["searchers"];
	if (package_searchers.valid())
	{
		package_searchers[2] = LuaSearcher;
		package_searchers[3] = sol::nil;
		package_searchers[4] = sol::nil;
	}

	LuaAPI::LuaBinding::ExecuteLuaBindings(mState.lua_state());

	LoadLuaGlobals(mState);

	for (const auto function : LuaAPI::sLuaInitFunctions)
	{
		function(mState.lua_state());
	}
	
	if (!DoLuaFile("luaspore/scripts/main"))
	{
		return;
	}

	for (const auto function : LuaAPI::sLuaPostInitFunctions)
	{
		function(mState.lua_state());
	}
	
	mLuaUpdate = mState["Update"];
}

LuaSpore::~LuaSpore()
{
	for (const auto function : LuaAPI::sLuaDisposeFunctions)
	{
		function(mState.lua_state());
	}
	mInstance = nullptr;
}

void LuaSpore::PostInit() const
{
	//this has a lifetime of the whole game, so we don't need to store the result to remove the update function
	Clock clock(Clock::Mode::Milliseconds);
	App::AddUpdateFunction([this, &clock]()
	{
		const double dt = clock.GetElapsedTime() / 1000.0;
		clock.Reset();
		clock.Start();
		Update(dt);
	});
}

void LuaSpore::Update(double dt) const
{
	if (!mLuaUpdate) return;
	const auto result = mLuaUpdate(dt);
}

lua_State* LuaSpore::GetLuaState() const
{
	return mState.lua_state();
}

bool LuaSpore::DoLuaFile(const char* file) const
{
	ModAPI::Log("DoLuaFile %s.lua", file);

	const auto& result = mState["require"](file);
	if (!result.valid())
	{
		const sol::error err = result;
		ModAPI::Log("DoLuaFile Error running lua file %s.lua:\n%s", file, err.what());
		return false;
	}
	return true;
}

sol::optional<sol::function> LuaSpore::LoadLuaBuffer(const char* package, const char* group, const char* instance)
{
	LuaSpore& lua_spore = GetLuaSpore();
	eastl::vector<char*> data;

	if (lua_spore.GetFolder(package))
	{
		eastl::string16 fullpath;
		fullpath.sprintf(u"%ls\\%hs\\%hs\\%hs.lua", lua_spore.GetLuaDevAbsolute().c_str(), package, group, instance);

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
	filename.sprintf("%s/%s/%s.lua", package, group, instance);

	return lua_spore.mState.load_buffer(reinterpret_cast<const char*>(data.data()), data.size(), filename.c_str(), sol::load_mode::text);
}

static eastl::string16 GetPackageNameFromDatabase(const Resource::Database* database)
{
	const eastl::string16 location = database->GetLocation();
	const auto location_length = location.size();
	if (location.find(u".package",  location_length - 8) != eastl::string16::npos)
	{
		const auto last_slash = location.find_last_of(u"/\\") + 1;
		auto package = location.substr(last_slash, location_length - last_slash - 8);

		return package;
	}
	return u"";
}

void LuaSpore::LocateLuaMods()
{
	Resource::IResourceManager::DatabaseList databases;
	auto count = ResourceManager.GetDatabaseList(databases);

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

Resource::Database* LuaSpore::GetDatabase(const char* dbpf_name)
{
	eastl::string16 dbpf_name_16;
	dbpf_name_16.sprintf(u"%hs", dbpf_name);
	const auto it = mLuaDatabases.find(dbpf_name_16.c_str());
	if (it != mLuaDatabases.end())
	{
		return it->second;
	}

	return nullptr;
}

bool LuaSpore::GetFolder(const char * folder_name) const
{
	eastl::string16 folder_name_16;
	folder_name_16.sprintf(u"%hs", folder_name);

	return mLuaFolders.count(folder_name_16.c_str()) == 1;
}

const eastl::string16& LuaSpore::GetLuaDevAbsolute()
{
	return mAbsoluteLuaDevDir;
}

#endif