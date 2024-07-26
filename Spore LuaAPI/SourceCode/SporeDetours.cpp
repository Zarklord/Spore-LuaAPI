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

#include <LuaSpore/SporeDetours.h>

using namespace LuaAPI;

static vector<SporeDetoursInstance*>& GetSporeDetours()
{
	static auto* detours = new vector<SporeDetoursInstance*>;
	return *detours;
}

void SporeDetours::RegisterInstance(SporeDetoursInstance* detour)
{
	GetSporeDetours().push_back(detour);	
}

void SporeDetours::Attach()
{
	for (const auto detour : GetSporeDetours())
		detour->AttachDetours();
}