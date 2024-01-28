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

#ifdef LUAAPI_DLL_EXPORT
#define LUAAPI __declspec(dllexport)
#define LUA_INTERNALPUBLIC public
#else
#define LUAAPI __declspec(dllimport)
#define LUA_INTERNALPUBLIC private
#endif

#include <mutex>
namespace LuaAPI
{
	class LUAAPI CriticalSection
	{
	public:
	    CriticalSection() {
	        InitializeCriticalSection(&m_cs);
	    }

	    ~CriticalSection() {
	        DeleteCriticalSection(&m_cs);
	    }

	    void lock() {
	        EnterCriticalSection(&m_cs);
	    }

	    void unlock() {
	        LeaveCriticalSection(&m_cs);
	    }
	private:
	    CRITICAL_SECTION m_cs;
	};
	extern LUAAPI CriticalSection LuaThreadGuard;
}

#define LUA_THREAD_SAFETY() const std::lock_guard _spore_lua_lock(LuaAPI::LuaThreadGuard)
