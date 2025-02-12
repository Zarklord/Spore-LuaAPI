#pragma once

void LuaLockCreate(lua_State* L);
void LuaLockDestroy(lua_State* L);
void LuaThreadCreate(lua_State* L, lua_State* L1);
void LuaThreadDestroy(lua_State* L, lua_State* L1);
void LuaLock(lua_State* L);
void LuaUnlock(lua_State* L);


#define luai_userstateopen(L) LuaLockCreate(L)
#define luai_userstateclose(L) LuaLockDestroy(L)

#define luai_userstatethread(L,L1) LuaThreadCreate(L, L1)  // Lua 5.1
#define luai_userstatefree(L,L1) LuaThreadDestroy(L, L1)  // Lua 5.1

#define lua_lock(L) LuaLock(L)
#define lua_unlock(L) LuaUnlock(L)
