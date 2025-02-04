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

#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>

static void LuaPrint(const char* str)
{
	ModAPI::Log("[Lua] %s", str);
}

static uint32_t LuaHash(const LuaFNVHash& value)
{
	return value;
}

void LuaSpore::LoadLuaGlobals(sol::state& s)
{
	s["LuaPrint"] = LuaPrint;
	s["fnv_id"] = LuaHash;
	s["GAMETYPE"] = static_cast<uint32_t>(ModAPI::GetGameType());
	s["LUA_VERSION"] = lua_version(s);
}

#endif