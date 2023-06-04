#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>

#include <Spore/Resource/cResourceManager.h>

#include <EASTL/hash_set.h>
#include <algorithm>
#include <cuchar>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

eastl::string16 GetModAPIRoot()
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

eastl::string16 GetPackageNameFromDatabase(const Resource::Database* database)
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

struct LuaSpore::tCheshireCat : public DefaultRefCounted, public App::IUpdatable
{
	tCheshireCat(LuaSpore* lua_spore)
	: mLuaSpore(lua_spore)
	, mClock(Clock::Mode::Milliseconds)
	{
		mAbsoluteLuaDevDir = GetModAPIRoot();
		mAbsoluteLuaDevDir.append_sprintf(u"\\%ls", luadev_folder);

		LocateLuaMods();
	}

	void LocateLuaMods()
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

	Resource::Database* GetDatabase(const eastl::string& dbpf_name)
	{
		eastl::string16 dbpf_name_16;
		dbpf_name_16.sprintf(u"%hs", dbpf_name.c_str());
		const auto it = mLuaDatabases.find(dbpf_name_16.c_str());
		if (it != mLuaDatabases.end())
		{
			return it->second;
		}

		return nullptr;
	}

	bool GetFolder(const eastl::string& folder_name) const
	{
		eastl::string16 folder_name_16;
		folder_name_16.sprintf(u"%hs", folder_name.c_str());

		return mLuaFolders.count(folder_name_16.c_str()) == 1;
	}

	const eastl::string16& GetLuaDevAbsolute()
	{
		return mAbsoluteLuaDevDir;
	}

	void Update() override
	{
		const double dt = mClock.GetElapsedTime() / 1000.0;
		mClock.Reset();
		mClock.Start();
		mLuaSpore->Update(dt);
	}

	virtual int AddRef() override
	{
		return DefaultRefCounted::AddRef();
	}

	virtual int Release() override
	{
		return DefaultRefCounted::Release();
	}
private:
	static constexpr const char16_t* luadev_folder = u"luadev";

	LuaSpore* mLuaSpore;

	eastl::string16 mAbsoluteLuaDevDir;

	Clock mClock;
	
	eastl::hash_map<eastl::string16, Resource::Database*> mLuaDatabases;
	eastl::hash_set<eastl::string16> mLuaFolders;
};

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

void* LuaStateAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	if (nsize == 0)
	{
		const char* data = static_cast<char*>(ptr);
		delete[] data;

		return nullptr;
	}
	else
	{
		const auto result = new char[nsize];
		if (ptr)
		{
			memcpy(result, ptr, std::min(nsize, osize));

			const char* data = static_cast<char*>(ptr);
			delete[] data;
		}
		return result;
	}
}

LuaSpore::LuaSpore()
: mLuaState(nullptr)
, mOpaquePtr(new tCheshireCat(this))
, mLuaTraceback(LUA_NOREF)
, mLuaUpdate(LUA_NOREF)
{
	mInstance = this;

	mOpaquePtr->AddRef();

	NewLuaState();
}

LuaSpore::~LuaSpore()
{
	mOpaquePtr->Release();
	mOpaquePtr = nullptr;

	CloseLuaState();

	mInstance = nullptr;
}
	

void LuaSpore::PostInit()
{
	//this has a lifetime of the whole game, so we don't need to store the result to remove the update function
	App::AddUpdateFunction(mOpaquePtr);
}

void LuaSpore::Update(double dt) const
{
	if (!mLuaState || mLuaUpdate == LUA_NOREF || mLuaUpdate == LUA_REFNIL) return;
	
	lua_rawgeti(mLuaState, LUA_REGISTRYINDEX, mLuaUpdate);
	lua_pushnumber(mLuaState, dt);
	const bool result = CallLuaFunction(1, 0);
}

int LuaPanic(lua_State* L)
{
	ModAPI::Log("LUA: RUN-TIME ERROR %s", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 0;
}

void LuaSpore::NewLuaState()
{
	CloseLuaState();

	mLuaState = lua_newstate(LuaStateAlloc, this);

	lua_atpanic(mLuaState, LuaPanic);

	luaL_openlibs(mLuaState);

	UpdatePackageSearchers();

	luaL_dostring(mLuaState, "_TRACEBACK = debug.traceback");
	lua_getglobal(mLuaState, "_TRACEBACK");
	mLuaTraceback = luaL_ref(mLuaState, LUA_REGISTRYINDEX);

	LoadLuaGlobals();
	
	if (!DoLuaFile("luaspore", "scripts", "main"))
	{
		return;
	}

	luaL_unref(mLuaState, LUA_REGISTRYINDEX, mLuaTraceback);
	lua_getglobal(mLuaState, "_TRACEBACK");
	mLuaTraceback = luaL_ref(mLuaState, LUA_REGISTRYINDEX);
	
	lua_getglobal(mLuaState, "Update");	
	mLuaUpdate = luaL_ref(mLuaState, LUA_REGISTRYINDEX);
}

void LuaSpore::CloseLuaState()
{
	if (!mLuaState) return;

	lua_gc(mLuaState, LUA_GCCOLLECT, 0);
	lua_close(mLuaState);
	mLuaState = nullptr;
}

void LuaSpore::ResetLuaState()
{
	NewLuaState();
}

int LuaSearcher(lua_State* L)
{
	const eastl::string modulename = luaL_checkstring(L, 1);

	const auto package_end = modulename.find_first_of(".\\/");
	const auto group_end = modulename.find_last_of(".\\/");

	if (package_end == eastl::string::npos)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is missing a package and group", modulename.c_str());
		lua_pushstring(L, errmsg.c_str());
	}
	else if (group_end == package_end)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is missing a package", modulename.c_str());
		lua_pushstring(L, errmsg.c_str());
	}

	const eastl::string package = modulename.substr(0, package_end);
	const eastl::string group = modulename.substr(package_end+1, group_end - (package_end+1));
	const eastl::string instance = modulename.substr(group_end+1);

	if (!LuaSpore::LoadLuaBuffer(L, package.c_str(), group.c_str(), instance.c_str()))
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is not found", modulename.c_str());
		lua_pushstring(L, errmsg.c_str());
	}
	return 1;
}

void LuaSpore::UpdatePackageSearchers() const
{
	lua_getglobal(mLuaState, "package");
	lua_getfield(mLuaState, -1, "searchers");
	lua_remove(mLuaState, -2);

	lua_pushnil(mLuaState);
	lua_rawseti(mLuaState, -2, 4); //package.searchers[4] = nil

	lua_pushnil(mLuaState);
	lua_rawseti(mLuaState, -2, 3); //package.searchers[3] = nil

	lua_pushcfunction(mLuaState, LuaSearcher);
	lua_rawseti(mLuaState, -2, 2); //package.searchers[2] = LuaSearcher

	lua_remove(mLuaState, -1);	
}

bool LuaSpore::CallLuaFunction(int narg, int nret) const
{
	const int base = lua_gettop(mLuaState) - narg;
	lua_rawgeti(mLuaState, LUA_REGISTRYINDEX, mLuaTraceback);
	lua_insert(mLuaState, base);
	const int status = lua_pcall(mLuaState, narg, nret, base);
	lua_remove(mLuaState, base);

	if (status != LUA_OK)
	{
		const char* str = lua_tostring(mLuaState, -1);
		ModAPI::Log("CallLuaFunction error: %s", str);
		lua_pop(mLuaState, 1);
		return false;
	}

	return true;
}

bool LuaSpore::DoLuaFile(const char* package, const char* group, const char* instance) const
{
	eastl::string filename;
	filename.sprintf("%s/%s/%s.lua", package, group, instance);

	ModAPI::Log("DoLuaFile %s", filename.c_str());

	bool success = LoadLuaBuffer(mLuaState, package, group, instance) == 1;
	if (success)
	{
		const int base = lua_gettop(mLuaState);
		lua_rawgeti(mLuaState, LUA_REGISTRYINDEX, mLuaTraceback);
		lua_insert(mLuaState, base);
		success = lua_pcall(mLuaState, 0, LUA_MULTRET, base) == LUA_OK;
		lua_remove(mLuaState, base);
	}

	if (!success)
	{
		const char * str = lua_tostring(mLuaState, -1);
		ModAPI::Log("DoLuaFile Error running lua file %s:\n%s", filename.c_str(), str);
		lua_pop(mLuaState, 1);
	}

	return success;
}

int LuaSpore::LoadLuaBuffer(lua_State* L, const char* package, const char* group, const char* instance)
{
	const auto& lua_spore = GetLuaSpore();
	
	eastl::vector<char*> data;

	if (lua_spore.mOpaquePtr->GetFolder(package))
	{
		eastl::string16 fullpath;
		fullpath.sprintf(u"%ls\\%hs\\%hs\\%hs.lua", lua_spore.mOpaquePtr->GetLuaDevAbsolute().c_str(), package, group, instance);

		if (!IO::File::Exists(fullpath.c_str()))
		{
			return 0;
		}

		const FileStreamPtr lua_file = new IO::FileStream(fullpath.c_str());
		if (!lua_file->Open(IO::AccessFlags::Read, IO::CD::OpenExisting))
		{
			return 0;
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
		const auto database = lua_spore.mOpaquePtr->GetDatabase(package);
		if (!database || !database->OpenRecord(lua_file, &record))
		{
			return 0;
		}

		const auto size = record->GetStream()->GetSize();
		data.resize(size);
		record->GetStream()->Read(data.data(), size);
		record->RecordClose();
	}

	eastl::string filename;
	filename.sprintf("%s/%s/%s.lua", package, group, instance);

	return luaL_loadbufferx(L, reinterpret_cast<const char*>(data.data()), data.size(), filename.c_str(), "t") == LUA_OK;
}

#endif