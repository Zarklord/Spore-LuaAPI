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

namespace LuaAPI
{
	class SporeDetoursInstance
	{
	public:
		SporeDetoursInstance() = default;
		virtual ~SporeDetoursInstance() = default;
		SporeDetoursInstance(const SporeDetoursInstance&) = delete;
	    SporeDetoursInstance& operator=(const SporeDetoursInstance&) = delete;
		SporeDetoursInstance(SporeDetoursInstance&&) = delete;
	    SporeDetoursInstance& operator=(SporeDetoursInstance&&) = delete;
		virtual void AttachDetours() = 0;
	};

	namespace SporeDetours
	{
		void RegisterInstance(SporeDetoursInstance* detour);
		void Attach();
	}
}

#define _SporeDetoursConcatImpl(x, y) x##y
#define _SporeDetoursConcat(x, y) _SporeDetoursConcatImpl(x, y)
#define _AddSporeDetoursImpl(class_name)									 \
	namespace SporeDetours  												 \
	{																		 \
		namespace  															 \
		{																	 \
			class class_name : public LuaAPI::SporeDetoursInstance			 \
			{																 \
			public:															 \
			    class_name() : LuaAPI::SporeDetoursInstance()				 \
				{															 \
					LuaAPI::SporeDetours::RegisterInstance(this);			 \
				}															 \
				void AttachDetours() override;						         \
			};																 \
			static class_name _SporeDetoursConcat(inst_, __COUNTER__){};	 \
		}																	 \
	}																		 \
	void SporeDetours::class_name::AttachDetours()
#define AddSporeDetours() _AddSporeDetoursImpl(_SporeDetoursConcat(SporeAttachDetoursCallback, __COUNTER__))