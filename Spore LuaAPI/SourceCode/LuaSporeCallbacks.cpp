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

#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSporeCallbacks.h>

using namespace LuaAPI;

static vector<LuaSporeCallbackInstance*>& GetLuaInitializers()
{
	static auto* initializers = new vector<LuaSporeCallbackInstance*>;
	return *initializers;
}

static vector<LuaSporeCallbackInstance*>& GetLuaPostInitializers()
{
	static auto* post_initializers = new vector<LuaSporeCallbackInstance*>;
	return *post_initializers;
}

static vector<LuaSporeCallbackInstance*>& GetLuaDisposers()
{
	static auto* disposers = new vector<LuaSporeCallbackInstance*>;
	return *disposers;
}

void LuaInitializers::RegisterInstance(LuaSporeCallbackInstance* initializer)
{
	GetLuaInitializers().push_back(initializer);	
}

void LuaPostInitializers::RegisterInstance(LuaSporeCallbackInstance* initializer)
{
	GetLuaPostInitializers().push_back(initializer);	
}

void LuaDisposers::RegisterInstance(LuaSporeCallbackInstance* disposer)
{
	GetLuaDisposers().push_back(disposer);	
}

void LuaInitializers::RunCallbacks(lua_State* L, bool is_main_state)
{
	for (const auto initializer : GetLuaInitializers())
		initializer->RunCallback(L, is_main_state);
}

void LuaPostInitializers::RunCallbacks(lua_State* L, bool is_main_state)
{
	for (const auto post_initializer : GetLuaPostInitializers())
		post_initializer->RunCallback(L, is_main_state);
}

void LuaDisposers::RunCallbacks(lua_State* L, bool is_main_state)
{
	for (const auto disposer : GetLuaDisposers())
		disposer->RunCallback(L, is_main_state);
}

#endif