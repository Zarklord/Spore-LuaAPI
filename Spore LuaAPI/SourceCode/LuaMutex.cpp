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

#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include "LuaSpore/LuaMutex.h"

MutexedLuaState::Impl::Impl(std::recursive_mutex* _mutex, lua_State* _state, lock_already_acquired)
: mutex(_mutex), state(_state) {}

MutexedLuaState::Impl::Impl(std::recursive_mutex* _mutex, lua_State* _state)
: Impl(_mutex, _state, lock_already_acquired{})
{
	mutex->lock();
}

MutexedLuaState::Impl::~Impl()
{
	mutex->unlock();
}

lua_State* MutexedLuaState::Impl::GetState() const
{
	return state;	
}

void MutexedLuaStates::Impl::SetState(size_t index, MutexedLuaState&& mutexed_lua_state)
{
	states.at(index) = std::move(mutexed_lua_state);
}

MutexedLuaState* MutexedLuaStates::Impl::begin()
{
	return states.data();
}

MutexedLuaState* MutexedLuaStates::Impl::end()
{
	return states.data() + states.size();
}

#endif