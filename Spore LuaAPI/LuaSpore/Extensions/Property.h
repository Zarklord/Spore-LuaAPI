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
#include <variant>

namespace Extensions
{
	class Property;
}

constexpr const Extensions::Property& GetPropertyExt(const App::Property& prop);
constexpr Extensions::Property& GetPropertyExt(App::Property& prop);
constexpr const Extensions::Property* GetPropertyExt(const App::Property* prop);
constexpr Extensions::Property* GetPropertyExt(App::Property* prop);

namespace Extensions
{
	class Property : public App::Property
	{
	public:
		enum ExtPropertyFlags : short
		{
			kPropertyFlagNone = 0x0,
			kPropertyFlagUnk1 = 0x1,
			kPropertyFlagUnk2 = 0x2,
			kPropertyFlagCleanup = 0x4,
			kPropertyFlagUnk8 = 0x8,
			kPropertyFlagPointer = 0x10,
			kPropertyFlagSkipDealloc = 0x20,
			kPropertyFlagUnk40 = 0x40,
		};

		operator App::Property*()
		{
			return this;
		}

		size_t GetItemCount() const
		{
			return mnItemCount;
		}
		size_t GetItemSize() const
		{
			return mnItemSize;
		}
		bool NeedsCleanup() const
		{
			return (mnFlags & kPropertyFlagCleanup) == kPropertyFlagCleanup;
		}
		bool IsArray() const
		{
			return (mnFlags & kPropertyFlagPointer) == kPropertyFlagPointer && GetItemCount() > 1;
		}
		bool OwnsMemory() const
		{
			return (mnFlags & kPropertyFlagSkipDealloc) != kPropertyFlagSkipDealloc;
		}
		short GetFlags() const
		{
			return mnFlags;
		}

		void* GetDataPointer() const
		{
			if ((mnFlags & (kPropertyFlagPointer | kPropertyFlagSkipDealloc)) != 0)
			{
				return mpData;
			}
			return nullptr;
		}

		void* GetValuePointer()
		{
			if ((mnFlags & (kPropertyFlagPointer | kPropertyFlagSkipDealloc)) != 0)
			{
				return mpData;
			}
			return this;
		}

		void Clear(const bool arg_0)
		{
			App::Property::Clear(arg_0);
		}

		struct smfx_array_string_8
		{
			const char * begin;
			const char * end;
		};

		struct array_string8
		{			
			const char* operator[](std::size_t idx) const
			{
				if (std::holds_alternative<smfx_array_string_8*>(dst))
				{
					return std::get<smfx_array_string_8*>(dst)[idx].begin;
				}
				else
				{
					return std::get<eastl::string8*>(dst)[idx].c_str();
				}
			}
			array_string8& operator=(smfx_array_string_8* strs)
			{
				dst = strs;
				return *this;
			}
			array_string8& operator=(eastl::string8* strs)
			{
				dst = strs;
				return *this;
			}
		private:
			std::variant<smfx_array_string_8*, eastl::string8*> dst;
		};
		
		static bool GetArrayString8(const App::PropertyList* pPropertyList, uint32_t propertyID, size_t& dstCount, array_string8& dst)
		{
			dstCount = 0;

			App::Property* prop;
			if (!pPropertyList->GetProperty(propertyID, prop))
				return false;

			const auto ext_prop = GetPropertyExt(prop);

			if (ext_prop->mnType != App::PropertyType::String8 || !ext_prop->IsArray())
				return false;

			if (ext_prop->GetItemSize() == 8)
			{
				dst = static_cast<smfx_array_string_8*>(ext_prop->GetValue());
			}
			else
			{
				dst = static_cast<eastl::string8*>(ext_prop->GetValue());
			}
			dstCount = ext_prop->GetItemCount();

			return true;
		}
	};

	inline size_t GetArrayItemCount(App::Property* prop)
	{
		return GetPropertyExt(prop)->GetItemCount();
	}
}

constexpr const Extensions::Property& GetPropertyExt(const App::Property& prop)
{ return static_cast<const Extensions::Property&>(prop); }

constexpr Extensions::Property& GetPropertyExt(App::Property& prop)
{ return static_cast<Extensions::Property&>(prop); }

constexpr const Extensions::Property* GetPropertyExt(const App::Property* prop)
{ return static_cast<const Extensions::Property*>(prop); }

constexpr Extensions::Property* GetPropertyExt(App::Property* prop)
{ return static_cast<Extensions::Property*>(prop); }

template <typename Handler>
inline bool sol_lua_check(sol::types<Extensions::Property>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<App::Property>(L, lua_absindex(L, index), handler);
}

inline Extensions::Property& sol_lua_get(sol::types<Extensions::Property>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return GetPropertyExt(sol::stack::get<App::Property>(L, lua_absindex(L, index)));
} 

inline int sol_lua_push(lua_State* L, Extensions::Property& prop)
{
	return sol::stack::push(L, static_cast<App::Property>(prop));
}

template <typename Handler>
inline bool sol_lua_check(sol::types<Extensions::Property*>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	tracking.use(1);
	return sol::stack::check<App::Property*>(L, lua_absindex(L, index), handler);
}

inline Extensions::Property* sol_lua_get(sol::types<Extensions::Property*>, lua_State* L, int index, sol::stack::record& tracking)
{
	tracking.use(1);
	return GetPropertyExt(sol::stack::get<App::Property*>(L, lua_absindex(L, index)));
} 

inline int sol_lua_push(lua_State* L, Extensions::Property* prop)
{
	return sol::stack::push(L, static_cast<App::Property*>(prop));
}