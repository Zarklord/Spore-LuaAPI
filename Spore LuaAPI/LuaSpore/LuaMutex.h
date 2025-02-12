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

#pragma once

#include <LuaSpore/LuaInternal.h>
#include <mutex>

class MutexedLuaStates;

class MutexedLuaState
{
	friend class MutexedLuaStates;
public:
	struct lock_already_acquired {};
private:
	struct Impl
	{
		LUAAPI Impl(const Impl&) = delete;
		LUAAPI Impl& operator=(const Impl&) = delete;
		LUAAPI Impl(Impl&& rhs) = delete;
		LUAAPI Impl& operator=(Impl&& rhs) = delete;
		
		LUAAPI Impl(std::recursive_mutex* _mutex, lua_State* _state, lock_already_acquired);
		LUAAPI Impl(std::recursive_mutex* _mutex, lua_State* _state);
		LUAAPI ~Impl();
		LUAAPI lua_State* GetState() const;
	private:
		std::recursive_mutex* mutex = nullptr;
		lua_State* state = nullptr;
	};
LUA_INTERNALPUBLIC:
	MutexedLuaState() = default;
	MutexedLuaState(std::recursive_mutex* mutex, lua_State* state, lock_already_acquired) : data(new Impl(mutex, state, lock_already_acquired{})) {}
	MutexedLuaState(std::recursive_mutex* mutex, lua_State* state) : data(new Impl(mutex, state)) {}
public:
	~MutexedLuaState() { delete data; data = nullptr; }

	MutexedLuaState(const MutexedLuaState&) = delete;
	MutexedLuaState& operator=(const MutexedLuaState&) = delete;
	MutexedLuaState(MutexedLuaState&& rhs)
	{
		*this = std::move(rhs);		
	}
	MutexedLuaState& operator=(MutexedLuaState&& rhs)
	{
		using std::swap;
		swap(data, rhs.data);
		return *this;
	}

	lua_State* lua_state() const { return data->GetState(); }
private:
	Impl* data = nullptr;
};

class MutexedLuaStates
{
private:
	struct Impl
	{
		LUAAPI Impl(const Impl&) = delete;
		LUAAPI Impl& operator=(const Impl&) = delete;
		LUAAPI Impl(Impl&& rhs) = delete;
		LUAAPI Impl& operator=(Impl&& rhs) = delete;

		LUAAPI Impl() = default;
		LUAAPI ~Impl() = default;
		LUAAPI void SetState(size_t index, MutexedLuaState&& mutexed_lua_state);
		
		LUAAPI MutexedLuaState* begin();
		LUAAPI MutexedLuaState* end();
	private:
		std::array<MutexedLuaState, 1 + LuaSporeConfiguration::NumThreadStates> states{};
	};
LUA_INTERNALPUBLIC:
	MutexedLuaStates() : data(new Impl()) {}
	void SetState(size_t index, MutexedLuaState&& mutexed_lua_state) const { data->SetState(index, std::move(mutexed_lua_state)); }
public:
	~MutexedLuaStates() { delete data; data = nullptr; }

	MutexedLuaStates(const MutexedLuaStates&) = delete;
	MutexedLuaStates& operator=(const MutexedLuaStates&) = delete;
	MutexedLuaStates(MutexedLuaStates&& rhs)
	{
		*this = std::move(rhs);		
	}
	MutexedLuaStates& operator=(MutexedLuaStates&& rhs)
	{
		using std::swap;
		swap(data, rhs.data);
		return *this;
	}

	struct iterator
	{
		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
	    using pointer           = MutexedLuaState*;

		iterator(pointer ptr) : m_ptr(ptr) {}

		lua_State* operator*() const { return m_ptr->lua_state(); }
	    lua_State* operator->() const { return m_ptr->lua_state(); }

	    iterator& operator++() { m_ptr++; return *this; }  
	    iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

	    friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr; }
	    friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr; }
	private:
		pointer m_ptr;
	};

	iterator begin() const { return {data->begin()}; }
	iterator end() const { return {data->end()}; }
private:
	Impl* data = nullptr;
};