#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class LuaSpore : public DefaultRefCounted, public App::IUpdatable
{
public:
	static void Initialize();
	static void Finalize();
	static LuaSpore& Get();
	static bool Exists();

private:
	static LuaSpore* mInstance;

	LuaSpore();
	~LuaSpore() override;

	virtual int AddRef() override;
	virtual int Release() override;
public:
	lua_State* GetLuaState() const { return mLuaState; }

	void PostInit();

	void Update() override;

	void ResetLuaState();

	bool CallLuaFunction(int narg, int nret) const;
	bool DoLuaFile(const char* group, const char* instance) const;

	static int LoadLuaBuffer(lua_State* L, const char* group, const char* instance);

private:
	void NewLuaState();
	void CloseLuaState();

	void UpdatePackageSearchers() const;
	
	void LoadLuaGlobals() const;

private:
	lua_State* mLuaState;
	Clock mClock;

	int mLuaTraceback;
	int mLuaUpdate;
};

LuaSpore& GetLuaSpore();