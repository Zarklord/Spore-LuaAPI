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

static std::vector<LuaSporeCallbackInstance*>& GetLuaInitializers()
{
	static std::vector<LuaSporeCallbackInstance*> initializers;
	return initializers;
}

static std::vector<LuaSporeCallbackInstance*>& GetLuaPostInitializers()
{
	static std::vector<LuaSporeCallbackInstance*> post_initializers;
	return post_initializers;
}

static std::vector<LuaSporeCallbackInstance*>& GetLuaDisposers()
{
	static std::vector<LuaSporeCallbackInstance*> disposers;
	return disposers;
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

void LuaInitializers::RunCallbacks(lua_State* L)
{
	for (const auto initializer : GetLuaInitializers())
		initializer->RunCallback(L);
}

void LuaPostInitializers::RunCallbacks(lua_State* L)
{
	for (const auto post_initializer : GetLuaPostInitializers())
		post_initializer->RunCallback(L);
}

void LuaDisposers::RunCallbacks(lua_State* L)
{
	for (const auto disposer : GetLuaDisposers())
		disposer->RunCallback(L);
}

#endif