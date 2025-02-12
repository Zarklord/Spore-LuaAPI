#include "pch.h"

#include <mutex>
#include <shared_mutex>

#include "lua.hpp"

class CriticalSectionMutex
{
public:
	CriticalSectionMutex()
	{
		InitializeCriticalSection(&critical_section);
	}
	~CriticalSectionMutex()
	{
		DeleteCriticalSection(&critical_section);
	}
	CriticalSectionMutex(const CriticalSectionMutex&) = delete;
	CriticalSectionMutex& operator=(const CriticalSectionMutex&) = delete;
	CriticalSectionMutex(CriticalSectionMutex&&) = default;
	CriticalSectionMutex& operator=(CriticalSectionMutex&&) = default;

	bool try_lock()
	{
		return TryEnterCriticalSection(&critical_section);
	}
	void lock()
	{
		EnterCriticalSection(&critical_section);
	}
	void unlock()
	{
		LeaveCriticalSection(&critical_section);
	}
private:
	CRITICAL_SECTION critical_section = {};
};

std::shared_mutex mLockLookupMutex;
static hash_map<lua_State*, CriticalSectionMutex*>* mLockLookup;

extern "C" void LuaLockCreate(lua_State* L)
{
	std::unique_lock lock(mLockLookupMutex);

	if (!mLockLookup) mLockLookup = new hash_map<lua_State*, CriticalSectionMutex*>;

	mLockLookup->insert(L).first->second = new CriticalSectionMutex;
}
extern "C" void LuaLockDestroy(lua_State* L)
{
	std::unique_lock lock(mLockLookupMutex);

	const auto it = mLockLookup->find(L);
	if (it == mLockLookup->end()) return;
	delete it->second;
	mLockLookup->erase(it);
}

extern "C" void LuaThreadCreate(lua_State* L, lua_State* L1)
{
	std::unique_lock lock(mLockLookupMutex);

	const auto it = mLockLookup->find(L);
	if (it == mLockLookup->end()) return;
	mLockLookup->insert(L1).first->second = it->second;
}

extern "C" void LuaThreadDestroy(lua_State* L, lua_State* L1)
{
	std::unique_lock lock(mLockLookupMutex);
	mLockLookup->erase(L1);
}

extern "C" void LuaLock(lua_State* L)
{
	std::shared_lock lock(mLockLookupMutex);
	const auto it = mLockLookup->find(L);
	if (it == mLockLookup->end()) return;
	it->second->lock();
}

extern "C" void LuaUnlock(lua_State* L)
{
	std::shared_lock lock(mLockLookupMutex);
	const auto it = mLockLookup->find(L);
	if (it == mLockLookup->end()) return;
	it->second->unlock();
}
