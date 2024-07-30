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
#include <LuaSpore/SporeDetours.h>

#include <LuaSpore/Extensions/Property.h>

static eastl::hash_map<std::uintptr_t, size_t>* sPropertyAllocationSize;
static PropertyListPtr sConstructedProperties = nullptr;
static uint32_t sConstructedPropertiesID = 0;

OnLuaInit(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	sConstructedProperties = new App::PropertyList();
	sPropertyAllocationSize = new eastl::hash_map<uintptr_t, size_t>();
}

OnLuaDispose(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;
	sConstructedProperties.reset();
	delete sPropertyAllocationSize;
	sPropertyAllocationSize = nullptr;
}

member_detour(Property_Clear, App::Property, void(bool arg_0))
{
	void detoured(bool arg_0) {
		const auto prop_ext = GetPropertyExt(this);
		if (prop_ext->IsArray() && prop_ext->OwnsMemory() && sPropertyAllocationSize)
		{
			const auto it = sPropertyAllocationSize->find(reinterpret_cast<std::uintptr_t>(prop_ext->GetDataPointer()));
			if (it != sPropertyAllocationSize->end())
			{
				sPropertyAllocationSize->erase(it);
			}
		}
		original_function(this, arg_0);
	}
};

AddSporeDetours()
{
	Property_Clear::attach(GetAddress(App::Property, Clear));
}

#define MakePropertyTypeFunction(function_name, property_type_function, args, arg_names)	  \
static bool function_name(const App::PropertyType prop_type, args)							  \
{																							  \
	switch(prop_type)																		  \
	{																						  \
		case App::PropertyType::Void:														  \
		{																					  \
			property_type_function<void>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Bool:														  \
		{																					  \
			property_type_function<bool>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Int32:														  \
		{																					  \
			property_type_function<int32_t>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::UInt32:														  \
		{																					  \
			property_type_function<uint32_t>(arg_names);									  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Float:														  \
		{																					  \
			property_type_function<float>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::String8:													  \
		{																					  \
			property_type_function<eastl::string8>(arg_names);								  \
			return true;																	  \
		}																					  \
		case App::PropertyType::String16:													  \
		{																					  \
			property_type_function<eastl::string16>(arg_names);								  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Key:														  \
		{																					  \
			property_type_function<ResourceKey>(arg_names);									  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Text:														  \
		{																					  \
			property_type_function<App::Property::TextProperty>(arg_names);					  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Vector2:													  \
		{																					  \
			property_type_function<Vector2>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Vector3:													  \
		{																					  \
			property_type_function<Vector3>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::ColorRGB:													  \
		{																					  \
			property_type_function<ColorRGB>(arg_names);									  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Vector4:													  \
		{																					  \
			property_type_function<Vector4>(arg_names);										  \
			return true;																	  \
		}																					  \
		case App::PropertyType::ColorRGBA:													  \
		{																					  \
			property_type_function<ColorRGBA>(arg_names);									  \
			return true;																	  \
		}																					  \
		case App::PropertyType::Transform:													  \
		{																					  \
			property_type_function<Transform>(arg_names);									  \
			return true;																	  \
		}																					  \
		case App::PropertyType::BBox:														  \
		{																					  \
			property_type_function<BoundingBox>(arg_names);									  \
			return true;																	  \
		}																					  \
	}																						  \
	return false;																			  \
}

static size_t GetPropertyTypeSize(const App::PropertyType prop_type)
{
	switch(prop_type)
	{
		case App::PropertyType::Void:
		{
			return 0;
		}
		case App::PropertyType::Bool:
		{
			return sizeof(bool);
		}
		case App::PropertyType::Int32:
		{
			return sizeof(int32_t);
		}
		case App::PropertyType::UInt32:
		{
			return sizeof(uint32_t);
		}
		case App::PropertyType::Float:
		{
			return sizeof(float);
		}
		case App::PropertyType::String8:
		{
			return sizeof(eastl::string8);
		}
		case App::PropertyType::String16:
		{
			return sizeof(eastl::string16);
		}
		case App::PropertyType::Key:
		{
			return sizeof(ResourceKey);
		}
		case App::PropertyType::Text:
		{
			return sizeof(App::Property::TextProperty);
		}
		case App::PropertyType::Vector2:
		{
			return sizeof(Vector2);
		}
		case App::PropertyType::Vector3:
		{
			return sizeof(Vector3);
		}
		case App::PropertyType::ColorRGB:
		{
			return sizeof(ColorRGB);
		}
		case App::PropertyType::Vector4:
		{
			return sizeof(Vector4);
		}
		case App::PropertyType::ColorRGBA:
		{
			return sizeof(ColorRGBA);
		}
		case App::PropertyType::Transform:
		{
			return sizeof(Transform);
		}
		case App::PropertyType::BBox:
		{
			return sizeof(BoundingBox);
		}
	}
	return 0;
}

template <typename T>
static void SetSingleProperty(App::Property* prop, const App::PropertyType prop_type, const short prop_flags = 0)
{
	if constexpr(sizeof(T) >= 0x10) assert(false);
	auto value = T{};
	prop->Set(prop_type, prop_flags, &value, sizeof(T), 1);
}
template <>
static void SetSingleProperty<void>(App::Property* prop, const App::PropertyType prop_type, const short prop_flags)
{
	prop->Set(prop_type, 0, nullptr, 0, 0);
}
template <>
static void SetSingleProperty<eastl::string8>(App::Property* prop, const App::PropertyType prop_type, const short prop_flags)
{
	if constexpr(sizeof(eastl::string8) >= 0x10) assert(false);
	auto value = eastl::string8{};
	prop->Set(prop_type, static_cast<short>(prop_flags | Extensions::Property::kPropertyFlagCleanup), &value, sizeof(eastl::string8), 1);
}
template <>
static void SetSingleProperty<eastl::string16>(App::Property* prop, const App::PropertyType prop_type, const short prop_flags)
{
	if constexpr(sizeof(eastl::string16) >= 0x10) assert(false);
	auto value = eastl::string16{};
	prop->Set(prop_type, static_cast<short>(prop_flags | Extensions::Property::kPropertyFlagCleanup), &value, sizeof(eastl::string16), 1);
}
MakePropertyTypeFunction(SetSinglePropertyType, SetSingleProperty, Args(App::Property* prop, const short prop_flags = 0), Args(prop, prop_type, prop_flags))

template <typename T>
static void SetPropertyArrayDefault(void* void_value_ptr, size_t start_index, size_t end_index)
{
	for (size_t i = start_index; i < end_index; ++i)
	{
		new(static_cast<T*>(void_value_ptr) + i) T();
	}
}
template <>
static void SetPropertyArrayDefault<void>(void* void_value_ptr, size_t start_index, size_t end_index)
{
}
MakePropertyTypeFunction(SetPropertyTypeArrayDefault, SetPropertyArrayDefault, Args(void* void_value_ptr, size_t start_index, size_t end_index), Args(void_value_ptr, start_index, end_index))

static void PropertySetType(Extensions::Property& property, const App::PropertyType prop_type, sol::optional<size_t> count)
{
	property.Clear(false);

	const auto prop_value_count = count.value_or(1);
	const auto prop_type_size = GetPropertyTypeSize(prop_type);
	if ((prop_type_size == 0 || (prop_value_count == 1 && prop_type_size < 0x10)) && SetSinglePropertyType(prop_type, property))
	{
		return;
	}

	auto* data = new char[prop_type_size * prop_value_count];
	SetPropertyTypeArrayDefault(prop_type, data, 0, prop_value_count);

	constexpr auto flags = static_cast<short>(
		Extensions::Property::kPropertyFlagCleanup |
		Extensions::Property::kPropertyFlagPointer
	);
	property.Set(prop_type, flags, data, prop_type_size, prop_value_count);
}

static App::Property& PropertyLuaConstructor(const App::PropertyType prop_type, sol::optional<size_t> count)
{
	{
		const App::Property prop;
		sConstructedProperties->SetProperty(sConstructedPropertiesID, &prop);
	}
	App::Property* prop;
	{
		sConstructedProperties->GetProperty(sConstructedPropertiesID, prop);
		++sConstructedPropertiesID;
	}
	PropertySetType(GetPropertyExt(*prop), prop_type, count);
	return *prop;
}

static void PropertyLuaReserve(Extensions::Property& property, size_t size)
{
	const size_t item_count = property.GetItemCount();

	const auto it = sPropertyAllocationSize->find(reinterpret_cast<std::uintptr_t>(property.GetDataPointer()));
	const size_t current_allocation_length = it != sPropertyAllocationSize->end() ? it->second : item_count;
	
	const bool owns_memory = property.OwnsMemory();
	if (current_allocation_length >= size && owns_memory) return;

	const size_t item_size = property.mnType == App::PropertyType::String8 ? sizeof(eastl::string8) : property.GetItemSize();

	void* old_mem = property.GetValuePointer();
	void* new_mem = new char[size * item_size];
	switch(property.mnType)
	{
		case App::PropertyType::String8:
		{
			if (property.GetItemSize() == 8)
			{
				for (size_t i = 0; i < item_count; ++i)
				{
					const auto& [old_begin, old_end] = static_cast<Extensions::Property::smfx_array_string_8*>(old_mem)[i];
					auto& new_value = static_cast<eastl::string8*>(new_mem)[i];
					new_value = eastl::string8(old_begin, old_end);
				}
			}
			else
			{
				for (size_t i = 0; i < item_count; ++i)
				{
					auto& old_value = static_cast<eastl::string8*>(old_mem)[i];
					auto& new_value = static_cast<eastl::string8*>(new_mem)[i];
					if (owns_memory)
					{
						new_value = {};
						old_value.swap(new_value);
					}
					else
					{
						new_value = old_value;
					}
				}
			}
			break;
		}
		case App::PropertyType::String16:
		{
			for (size_t i = 0; i < item_count; ++i)
			{
				auto& old_value = static_cast<eastl::string16*>(old_mem)[i];
				auto& new_value = static_cast<eastl::string16*>(new_mem)[i];
				if (owns_memory)
				{
					new_value = {};
					old_value.swap(new_value);
				}
				else
				{
					new_value = old_value;
				}
			}
			break;
		}
		case App::PropertyType::Text:
		case App::PropertyType::Bool:
		case App::PropertyType::Int32:
		case App::PropertyType::UInt32:
		case App::PropertyType::Float:
		case App::PropertyType::Key:
		case App::PropertyType::Vector2:
		case App::PropertyType::Vector3:
		case App::PropertyType::ColorRGB:
		case App::PropertyType::Vector4:
		case App::PropertyType::ColorRGBA:
		case App::PropertyType::Transform:
		case App::PropertyType::BBox:
		{
			memcpy(new_mem, old_mem, item_size*item_count);
			break;
		}
		default:
		{
			break;
		}
	}
	(*sPropertyAllocationSize)[reinterpret_cast<std::uintptr_t>(new_mem)] = size;
	const auto new_flags = static_cast<short>((property.mnFlags & ~Extensions::Property::kPropertyFlagSkipDealloc)
		| Extensions::Property::kPropertyFlagCleanup | Extensions::Property::kPropertyFlagPointer);
	property.Set(property.mnType, new_flags, new_mem, item_size, item_count);
	delete static_cast<char*>(old_mem);
}

static void PropertyLuaSet(sol::this_state L, Extensions::Property& property, size_t index, sol::stack_object value)
{
	const sol::state_view s(L);
	const auto idx = index - 1;

	const size_t item_count = property.GetItemCount();

	if (idx > item_count || !property.OwnsMemory())
	{
		PropertyLuaReserve(property, index);
		SetPropertyTypeArrayDefault(property.mnType, property.GetValuePointer(), item_count, index);
	}

	void* void_value_ptr = property.GetValuePointer();
	switch(property.mnType)
	{
		case App::PropertyType::String8:
		{
			auto* value_ptr = static_cast<eastl::string8*>(void_value_ptr);
			const auto new_str = value.as<sol::string_view>();
			value_ptr[idx].assign(new_str.data(), new_str.length());
		}
		case App::PropertyType::String16:
		{
			auto* value_ptr = static_cast<eastl::string16*>(void_value_ptr);
			const auto new_str = value.as<sol::u16string_view>();
			value_ptr[idx].assign(new_str.data(), new_str.length());
			break;
		}
		case App::PropertyType::Bool:
		{
			const auto new_bool= value.as<bool>();
			const auto type_size = property.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, &new_bool, type_size);
			break;
		}
		case App::PropertyType::Int32:
		{
			const auto new_int32 = value.as<int32_t>();
			const auto type_size = property.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, &new_int32, type_size);
			break;
		}
		case App::PropertyType::UInt32:
		{
			const auto new_uint32 = value.as<uint32_t>();
			const auto type_size = property.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, &new_uint32, type_size);
			break;
		}
		case App::PropertyType::Float:
		{
			const auto new_float = value.as<float>();
			const auto type_size = property.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, &new_float, type_size);
			break;
		}
		case App::PropertyType::Text:
		case App::PropertyType::Key:
		case App::PropertyType::Vector2:
		case App::PropertyType::Vector3:
		case App::PropertyType::ColorRGB:
		case App::PropertyType::Vector4:
		case App::PropertyType::ColorRGBA:
		case App::PropertyType::Transform:
		case App::PropertyType::BBox:
		{
			const auto type_size = property.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, value.as<void*>(), type_size);
			break;
		}
		default:
		{
			break;
		}
	}
}

static sol::object PropertyLuaGet(sol::this_state L, Extensions::Property& property, sol::optional<size_t> index)
{
	const sol::state_view s(L);
	const auto idx = index.value_or(1) - 1;
	
	const size_t num_values = property.GetItemCount();

	if (idx > num_values)
	{
		return sol::nil;
	}

	void* void_value_ptr = property.GetValuePointer();
	
	switch(property.mnType)
	{
		case App::PropertyType::Bool:
		{
			const auto* value_ptr = static_cast<const bool*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Int32:
		{
			const auto* value_ptr = static_cast<const int32_t*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::UInt32:
		{
			const auto* value_ptr = static_cast<const uint32_t*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Float:
		{
			const auto* value_ptr = static_cast<const float*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::String8:
		{
			if (property.GetItemSize() == 8)
			{
				const auto* value_ptr = static_cast<const Extensions::Property::smfx_array_string_8*>(void_value_ptr);
				return sol::make_object(L, sol::string_view(value_ptr->begin, value_ptr->end - value_ptr->begin));
			}
			else
			{
				const auto* value_ptr = static_cast<const eastl::string8*>(void_value_ptr);
				return sol::make_object(L, value_ptr[idx]);
			}
		}
		case App::PropertyType::String16:
		{
			const auto* value_ptr = static_cast<const eastl::string16*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Key:
		{
			auto* value_ptr = static_cast<ResourceKey*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::Text:
		{
			auto* value_ptr = static_cast<App::Property::TextProperty*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::Vector2:
		{
			auto* value_ptr = static_cast<Vector2*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::Vector3:
		{
			auto* value_ptr = static_cast<Vector3*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::ColorRGB:
		{
			auto* value_ptr = static_cast<ColorRGB*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::Vector4:
		{
			auto* value_ptr = static_cast<Vector4*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::ColorRGBA:
		{
			auto* value_ptr = static_cast<ColorRGBA*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::Transform:
		{
			auto* value_ptr = static_cast<Transform*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		case App::PropertyType::BBox:
		{
			auto* value_ptr = static_cast<BoundingBox*>(void_value_ptr);
			return sol::make_object(L, value_ptr + idx);
		}
		default:
		{
			return sol::make_object(L, sol::nil);
		}
	}
}

static size_t PropertyLuaLength(const Extensions::Property& property)
{
	return property.IsArray() ? property.GetItemCount() : 1u;
}

static void PropertyLuaCopyFrom(Extensions::Property& base_prop, Extensions::Property& copy_prop)
{
	base_prop.Clear(false);
	
	constexpr short clear_flags = Extensions::Property::kPropertyFlagSkipDealloc | Extensions::Property::kPropertyFlagPointer | Extensions::Property::kPropertyFlagUnk8;
	constexpr short add_flags =  Extensions::Property::kPropertyFlagCleanup;
	const auto new_flags = static_cast<short>((copy_prop.mnFlags & ~clear_flags) | add_flags);

	static auto type_size = GetPropertyTypeSize(copy_prop.mnType);
	static auto type_count = PropertyLuaLength(copy_prop);

	char* copy_memory = static_cast<char*>(copy_prop.GetValuePointer());
	char* value_memory = new char[type_size* type_count];

	switch(copy_prop.mnType)
	{
		case App::PropertyType::String8:
		{
			const auto* copy_string8_memory = reinterpret_cast<eastl::string8*>(copy_memory);
			auto* value_string8_memory = reinterpret_cast<eastl::string8*>(value_memory);
			for (size_t i = 0; i < type_count; ++i)
			{
				new(value_string8_memory + i) eastl::string8(copy_string8_memory[i]);
			}
			break;
		}
		case App::PropertyType::String16:
		{
			const auto* copy_string16_memory = reinterpret_cast<eastl::string16*>(copy_memory);
			auto* value_string16_memory = reinterpret_cast<eastl::string16*>(value_memory);
			for (size_t i = 0; i < type_count; ++i)
			{
				new(value_string16_memory + i) eastl::string16(copy_string16_memory[i]);
			}
			break;
		}
		case App::PropertyType::Text:
		case App::PropertyType::Bool:
		case App::PropertyType::Int32:
		case App::PropertyType::UInt32:
		case App::PropertyType::Float:
		case App::PropertyType::Key:
		case App::PropertyType::Vector2:
		case App::PropertyType::Vector3:
		case App::PropertyType::ColorRGB:
		case App::PropertyType::Vector4:
		case App::PropertyType::ColorRGBA:
		case App::PropertyType::Transform:
		case App::PropertyType::BBox:
		{
			memcpy(value_memory, copy_memory, type_size*type_count);
			break;
		}
		default:
		{
			break;
		}
	}

	base_prop.Set(
		copy_prop.mnType,
		new_flags,
		value_memory,
		type_size,
		type_count
	);
	
	if ((base_prop.mnFlags & Extensions::Property::kPropertyFlagPointer) == 0)
	{
		delete[] value_memory;
	}
}

static void PropertyLuaReferenceFrom(Extensions::Property& base_prop, Extensions::Property& reference_prop)
{
	base_prop.Clear(false);
	
	constexpr short clear_flags = Extensions::Property::kPropertyFlagCleanup | Extensions::Property::kPropertyFlagUnk8;
	constexpr short add_flags = Extensions::Property::kPropertyFlagPointer | Extensions::Property::kPropertyFlagSkipDealloc;
	const auto new_flags = static_cast<short>((reference_prop.mnFlags & ~clear_flags) | add_flags);

	static auto type_size = GetPropertyTypeSize(reference_prop.mnType);
	static auto type_count = PropertyLuaLength(reference_prop);

	base_prop.Set(
		reference_prop.mnType,
		new_flags,
		reference_prop.GetValuePointer(),
		type_size,
		type_count
	);
}

OnLuaInit(sol::state_view s, bool is_main_state)
{	
	s.new_usertype<App::Property>(
		"Property",
		sol::call_constructor, sol::factories(PropertyLuaConstructor	),
		"IsArray", [](const Extensions::Property& property)
		{
			return property.IsArray();
		},
		"GetPropertyType", [](const Extensions::Property& property)
		{
			return property.mnType;
		},
		sol::meta_function::length, PropertyLuaLength,
		"GetItemCount", PropertyLuaLength,
		sol::meta_function::index, PropertyLuaGet,
		"Get", PropertyLuaGet,
		sol::meta_function::new_index, PropertyLuaSet,
		"Set", [](sol::this_state L, Extensions::Property& property, sol::stack_object value)
		{
			PropertyLuaSet(L, property, 1, value);
		},
		"Reserve", PropertyLuaReserve,
		"SetType", PropertySetType,
		"CopyFrom", PropertyLuaCopyFrom,
		"ReferenceFrom", PropertyLuaReferenceFrom,
		sol::meta_function::to_string, [](const Extensions::Property& property)
		{
			return string().sprintf("App::Property (%p)", &property);
		}
	);
}

#endif