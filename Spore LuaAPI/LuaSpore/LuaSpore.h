#pragma once

#include <LuaSpore\LuaInternal.h>

class LUAAPI LuaSpore
{
public:
	static void Initialize();
	static void Finalize();
	static LuaSpore& Get();
	static bool Exists();
	
	LuaSpore(const LuaSpore&) = delete;
    LuaSpore& operator=(const LuaSpore&) = delete;
	LuaSpore(LuaSpore&&) = delete;
    LuaSpore& operator=(LuaSpore&&) = delete;
private:
	static LuaSpore* mInstance;

	LuaSpore();
	~LuaSpore();
public:
	[[nodiscard]] lua_State* GetLuaState() const { return mLuaState; }

	void PostInit();

	void Update(double dt) const;

	void ResetLuaState();

	bool CallLuaFunction(int narg, int nret) const;
	bool DoLuaFile(const char* package, const char* group, const char* instance) const;

	static int LoadLuaBuffer(lua_State* L, const char* package, const char* group, const char* instance);

private:
	void NewLuaState();
	void CloseLuaState();

	void UpdatePackageSearchers() const;
	
	void LoadLuaGlobals() const;

private:
	lua_State* mLuaState;
	
	struct tCheshireCat; 
	tCheshireCat* mOpaquePtr;

	int mLuaTraceback;
	int mLuaUpdate;
};

extern LUAAPI LuaSpore& GetLuaSpore();