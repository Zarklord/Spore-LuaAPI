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
#include "LuaMutex.h"

template <typename T>
class LuaMultiReference
{
	using reference = pair<lua_State*, T>;
	vector<reference>* references = nullptr;
public:
	void clear(const MutexedLuaState& mutexed_lua_state)
	{
		clear(mutexed_lua_state.lua_state());
	}

	void clear(lua_State* L)
	{
		if (!references) return;

		auto end = references->end();
		auto it = eastl::remove_if(references->begin(), end, [L](reference ref)
		{
			return ref.first == L;
		});

		if (it != end)
		{
			references->erase(it, end);

			if (references->empty())
			{
				delete references;
				references = nullptr;
			}
		}
	}

	void set(const MutexedLuaState& mutexed_lua_state, T&& value)
	{
		set(mutexed_lua_state.lua_state(), std::move(value));
	}

	void set(lua_State* L, T&& value)
	{
		if (!references) references = new vector<reference>;
		references->push_back(pair{L, std::move(value)});
	}

	T* get(const MutexedLuaState& mutexed_lua_state)
	{
		return get(mutexed_lua_state.lua_state());
	}

	T* get(lua_State* L)
	{
		if (!references) return nullptr;
		for (auto& [lua_state, value] : *references)
		{
			if (lua_state == L)
			{
				return &value;
			}
		}
		return nullptr;
	}

	void reset()
	{
		delete references;
		references = nullptr;
	}

	bool valid() const
	{
		return references;
	}

	operator bool() const
	{
		return valid();
	}
};
