#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT


#include "LuaSpore.h"

#include <algorithm>

#include "Spore/Resource/cResourceManager.h"

LuaSpore* LuaSpore::mInstance = nullptr;
void LuaSpore::Initialize()
{
	mInstance = new LuaSpore();
	mInstance->AddRef();
}

void LuaSpore::Finalize()
{
	mInstance->Release();
	mInstance = nullptr;
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

int LuaSpore::AddRef()
{
	return DefaultRefCounted::AddRef();
}

int LuaSpore::Release()
{
	return DefaultRefCounted::Release();
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
, mClock(Clock::Mode::Milliseconds)
, mLuaTraceback(LUA_NOREF)
, mLuaUpdate(LUA_NOREF)
{
	NewLuaState();
}

LuaSpore::~LuaSpore()
{
	CloseLuaState();
}
	

void LuaSpore::PostInit()
{
	//this has a lifetime of the whole game, so we don't need to store the result to remove the update function
	App::AddUpdateFunction(this);
}

void LuaSpore::Update()
{
	const int milliseconds = mClock.GetElapsedTime();
	mClock.Reset();
	mClock.Start();

	if (!mLuaState || mLuaUpdate == LUA_NOREF || mLuaUpdate == LUA_REFNIL) return;
	
	lua_rawgeti(mLuaState, LUA_REGISTRYINDEX, mLuaUpdate);
	lua_pushnumber(mLuaState, static_cast<double>(milliseconds) / 1000.0);
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
	
	if (!DoLuaFile("scripts", "main"))
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

int LuaDBPFSearcher(lua_State* L)
{
	const eastl::string modulename = luaL_checkstring(L, 1);
	const auto first_sep = modulename.find_first_of(".\\/");
	const auto last_sep = modulename.find_last_of(".\\/");
	if (first_sep == eastl::string::npos)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' is missing a directory", modulename.c_str());
		lua_pushstring(L, errmsg.c_str());
	}
	else if (first_sep != last_sep)
	{
		eastl::string errmsg;
		errmsg.sprintf("file '%s' can only have one directory", modulename.c_str());
		lua_pushstring(L, errmsg.c_str());
	}
	else
	{
		const eastl::string group = modulename.substr(0, first_sep);
		const eastl::string instance = modulename.substr(first_sep +  1);
		if (!LuaSpore::LoadLuaBuffer(L, group.c_str(), instance.c_str()))
		{
			eastl::string errmsg;
			errmsg.sprintf("file '%s' is not found", modulename.c_str());
			lua_pushstring(L, errmsg.c_str());
		}
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

	lua_pushcfunction(mLuaState, LuaDBPFSearcher);
	lua_rawseti(mLuaState, -2, 2); //package.searchers[2] = LuaDBPFSearcher

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

bool LuaSpore::DoLuaFile(const char* group, const char* instance) const
{
	eastl::string filename = group;
	filename += "/";
	filename += instance;
	filename += ".lua";
	ModAPI::Log("DoLuaFile %s", filename.c_str());

	bool success = LoadLuaBuffer(mLuaState, group, instance) == 1;
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

int LuaSpore::LoadLuaBuffer(lua_State* L, const char* group, const char* instance)
{
	const ResourceKey lua_file(id(instance), id("lua"), id(group));

	Resource::IRecord* pRecord;
	const auto database = ResourceManager.FindDatabase(lua_file);
	if (!database || !database->OpenRecord(lua_file, &pRecord))
	{
		return 0;
	}

	pRecord->GetStream()->SetPosition(0);
	const auto size = pRecord->GetStream()->GetSize();
	eastl::vector<char*> data(size);
	pRecord->GetStream()->Read(data.data(), size);
	pRecord->RecordClose();

	eastl::string filename = group;
	filename += "/";
	filename += instance;
	filename += ".lua";

	return luaL_loadbufferx(L, reinterpret_cast<const char*>(data.data()), size, filename.c_str(), "t") == LUA_OK;
}

#endif