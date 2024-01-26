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

#include <LuaSpore/LuaInternal.h>
#include <LuaSpore/DefaultIncludes.h>

namespace LuaAPI
{
	class LUAAPI LuaSporeCallbackInstance
	{
	public:
		LuaSporeCallbackInstance() = default;
		virtual ~LuaSporeCallbackInstance() = default;
		LuaSporeCallbackInstance(const LuaSporeCallbackInstance&) = delete;
	    LuaSporeCallbackInstance& operator=(const LuaSporeCallbackInstance&) = delete;
		LuaSporeCallbackInstance(LuaSporeCallbackInstance&&) = delete;
	    LuaSporeCallbackInstance& operator=(LuaSporeCallbackInstance&&) = delete;
	LUA_INTERNALPUBLIC:
		virtual void RunCallback(lua_State* L) = 0;
	};

	namespace LuaInitializers
	{
		void LUAAPI RegisterInstance(LuaSporeCallbackInstance* binding);
#ifdef LUAAPI_DLL_EXPORT
		void RunCallbacks(lua_State* L);
#endif
	}

	namespace LuaPostInitializers
	{
		void LUAAPI RegisterInstance(LuaSporeCallbackInstance* binding);
#ifdef LUAAPI_DLL_EXPORT
		void RunCallbacks(lua_State* L);
#endif
	}

	namespace LuaDisposers
	{
		void LUAAPI RegisterInstance(LuaSporeCallbackInstance* binding);
#ifdef LUAAPI_DLL_EXPORT
		void RunCallbacks(lua_State* L);
#endif
	}
}

#define _LuaSporeCallbackConcatImpl(x, y) x##y
#define _LuaSporeCallbackConcat(x, y) _LuaSporeCallbackConcatImpl(x, y)

#define _OnLuaInitImpl(class_name, binding_arg)								 \
	namespace LuaInitializers  												 \
	{																		 \
		namespace  															 \
		{																	 \
			class class_name : public LuaAPI::LuaSporeCallbackInstance		 \
			{																 \
			public:															 \
			    class_name() : LuaAPI::LuaSporeCallbackInstance()			 \
				{															 \
					LuaAPI::LuaInitializers::RegisterInstance(this);		 \
				}															 \
				void RunCallback(lua_State* L) override						 \
				{															 \
					CallbackFunction(L);									 \
				}															 \
				void CallbackFunction(binding_arg);							 \
			};																 \
			static class_name _LuaSporeCallbackConcat(inst_, __COUNTER__){}; \
		}																	 \
	}																		 \
	void LuaInitializers::class_name::CallbackFunction(binding_arg)
#define OnLuaInit(binding_arg) _OnLuaInitImpl(_LuaSporeCallbackConcat(LuaInitCallback_, __COUNTER__), binding_arg)

#define _OnLuaPostInitImpl(class_name, binding_arg)							 \
	namespace LuaPostInitializers  											 \
	{																		 \
		namespace  															 \
		{																	 \
			class class_name : public LuaAPI::LuaSporeCallbackInstance		 \
			{																 \
			public:															 \
			    class_name() : LuaAPI::LuaSporeCallbackInstance()			 \
				{															 \
					LuaAPI::LuaPostInitializers::RegisterInstance(this);	 \
				}															 \
				void RunCallback(lua_State* L) override						 \
				{															 \
					CallbackFunction(L);									 \
				}															 \
				void CallbackFunction(binding_arg);							 \
			};																 \
			static class_name _LuaSporeCallbackConcat(inst_, __COUNTER__){}; \
		}																	 \
	}																		 \
	void LuaPostInitializers::class_name::CallbackFunction(binding_arg)
#define OnLuaPostInit(binding_arg) _OnLuaPostInitImpl(_LuaSporeCallbackConcat(LuaPostInitCallback_, __COUNTER__), binding_arg)

#define _OnLuaDisposeImpl(class_name, binding_arg)							 \
	namespace LuaDisposers  												 \
	{																		 \
		namespace  															 \
		{																	 \
			class class_name : public LuaAPI::LuaSporeCallbackInstance		 \
			{																 \
			public:															 \
			    class_name() : LuaAPI::LuaSporeCallbackInstance()			 \
				{															 \
					LuaAPI::LuaDisposers::RegisterInstance(this);		     \
				}															 \
				void RunCallback(lua_State* L) override						 \
				{															 \
					CallbackFunction(L);									 \
				}															 \
				void CallbackFunction(binding_arg);							 \
			};																 \
			static class_name _LuaSporeCallbackConcat(inst_, __COUNTER__){}; \
		}																	 \
	}																		 \
	void LuaDisposers::class_name::CallbackFunction(binding_arg)
#define OnLuaDispose(binding_arg) _OnLuaDisposeImpl(_LuaSporeCallbackConcat(LuaDisposeCallback_, __COUNTER__), binding_arg)