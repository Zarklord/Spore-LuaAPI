#pragma once

#include <LuaSpore\LuaInternal.h>
#include <LuaSpore\DefaultIncludes.h>

class LuaSpore
{
public:
	LUAAPI [[nodiscard]] lua_State* GetLuaState() const;
	LUAAPI [[nodiscard]] sol::state_view GetState() const
	{
		return sol::state_view(GetLuaState());
	}
	LUAAPI bool DoLuaFile(const char* file) const;
	LUAAPI static sol::optional<sol::function> LoadLuaBuffer(const char* package, const char* group, const char* instance);

LUA_INTERNALPUBLIC:
	static void Initialize();
	static void Finalize();
	static LuaSpore& Get();
	static bool Exists();
	
	LuaSpore(const LuaSpore&) = delete;
    LuaSpore& operator=(const LuaSpore&) = delete;
	LuaSpore(LuaSpore&&) = delete;
    LuaSpore& operator=(LuaSpore&&) = delete;
	
	void PostInit() const;
private:
	static LuaSpore* mInstance;

	LuaSpore();
	~LuaSpore();
private:
	void Update(double dt) const;

	static void LoadLuaGlobals(sol::state& s);

	void LocateLuaMods();
	Resource::Database* GetDatabase(const eastl::string& dbpf_name);
	bool GetFolder(const eastl::string& folder_name) const;
	const eastl::string16& GetLuaDevAbsolute();

	static constexpr const char16_t* luadev_folder = u"luadev";

	sol::state mState;

	sol::function mLuaTraceback;
	sol::function mLuaUpdate;

	eastl::string16 mAbsoluteLuaDevDir;
	
	eastl::hash_map<eastl::string16, Resource::Database*> mLuaDatabases;
	eastl::hash_set<eastl::string16> mLuaFolders;
};

extern LUAAPI LuaSpore& GetLuaSpore();