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

#include <tracy/Tracy.hpp>

namespace TracyUtil
{
#ifdef TRACY_ON_DEMAND
	inline thread_local uint32_t tracy_counter = 0;
	inline thread_local bool tracy_active = false;
#endif

	inline void TracyZonePush(const tracy::SourceLocationData* srcloc)
	{
		using namespace tracy;
#ifdef TRACY_ENABLE
#ifdef TRACY_ON_DEMAND
		const auto zone_count = tracy_counter++;
		if(zone_count != 0 && !tracy_active) return;
		tracy_active = TracyIsConnected;
		if(!tracy_active) return;
#endif
		TracyQueuePrepare(QueueType::ZoneBegin)
		MemWrite(&item->zoneBegin.time, Profiler::GetTime());
		MemWrite(&item->zoneBegin.srcloc, srcloc);
		TracyQueueCommit(zoneBeginThread);
#endif
	}

	inline void TracyZonePop()
	{
		using namespace tracy;
#ifdef TRACY_ENABLE
#ifdef TRACY_ON_DEMAND
		assert(tracy_counter != 0);
		tracy_counter--;
		if(!tracy_active) return;
		if(!TracyIsConnected)
		{
			tracy_active = false;
			return;
		}
#endif
		TracyQueuePrepare(QueueType::ZoneEnd);
		MemWrite(&item->zoneEnd.time, Profiler::GetTime());
		TracyQueueCommit(zoneEndThread);
#endif
	}
}

#define ZonePush static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { nullptr, TracyFunction,  TracyFile, (uint32_t)TracyLine, 0 }; TracyUtil::TracyZonePush(&TracyConcat(__tracy_source_location,TracyLine))
#define ZonePushN(name)  static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { name, TracyFunction,  TracyFile, (uint32_t)TracyLine, 0 }; TracyUtil::TracyZonePush(&TracyConcat(__tracy_source_location,TracyLine))
#define ZonePop TracyUtil::TracyZonePop()