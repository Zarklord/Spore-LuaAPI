#include <pch.h>

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\LuaBinding.h>

#include <LuaSpore\Extensions\Property.h>

static eastl::hash_map<std::uintptr_t, size_t> PropertyAllocationSize;

static void PropertyLuaReserve(App::Property& property, size_t size)
{
	auto& property_ext = GetPropertyExt(property);
	const size_t item_count = property_ext.GetItemCount();

	const auto it = PropertyAllocationSize.find(reinterpret_cast<std::uintptr_t>(property_ext.GetDataPointer()));
	const size_t current_allocation_length = it != PropertyAllocationSize.end() ? it->second : item_count;
	
	const bool owns_memory = property_ext.OwnsMemory();
	if (current_allocation_length >= size && owns_memory) return;

	const size_t item_size = property_ext.mnType == App::PropertyType::String8 ? sizeof(eastl::string8) : property_ext.GetItemSize();

	void* old_mem = property_ext.GetValuePointer();
	void* new_mem = new char[size * item_size];
	switch(property_ext.mnType)
	{
		case App::PropertyType::String8:
		{
			if (property_ext.GetItemSize() == 8)
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
		{
			for (size_t i = 0; i < item_count; ++i)
			{
				const auto& old_value = static_cast<LocalizedString*>(old_mem)[i];
				auto& new_value = static_cast<LocalizedString*>(new_mem)[i];
				new_value = old_value;
			}
			break;
		}
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
	PropertyAllocationSize[reinterpret_cast<std::uintptr_t>(new_mem)] = size;
	const auto new_flags = static_cast<short>((property.mnFlags & ~Extensions::Property::kPropertyFlagSkipDealloc)
		| Extensions::Property::kPropertyFlagCleanup | Extensions::Property::kPropertyFlagPointer);
	property.Set(property.mnType, new_flags, new_mem, item_size, item_count);
}

static void PropertyLuaSet(sol::this_state L, App::Property& property, size_t index, sol::stack_object value)
{
	const sol::state_view s(L);
	const auto idx = index - 1;

	auto& property_ext = GetPropertyExt(property);
	const size_t item_count = property_ext.GetItemCount();

	if (idx > item_count || !property_ext.OwnsMemory())
	{
		PropertyLuaReserve(property, index);
		
		void* void_value_ptr = property_ext.GetValuePointer();
		for (size_t i = item_count; i < index; ++i)
		{
			switch(property_ext.mnType)
			{
				case App::PropertyType::Bool:
				{
					auto* value_ptr = static_cast<bool*>(void_value_ptr);
					value_ptr[idx] = bool{};
					break;
				}
				case App::PropertyType::Int32:
				{
					auto* value_ptr = static_cast<int32_t*>(void_value_ptr);
					value_ptr[idx] = int32_t{};
					break;
				}
				case App::PropertyType::UInt32:
				{
					auto* value_ptr = static_cast<uint32_t*>(void_value_ptr);
					value_ptr[idx] = uint32_t{};
					break;
				}
				case App::PropertyType::Float:
				{
					auto* value_ptr = static_cast<float*>(void_value_ptr);
					value_ptr[idx] = float{};
					break;
				}
				case App::PropertyType::String8:
				{
					auto* value_ptr = static_cast<eastl::string8*>(void_value_ptr);
					value_ptr[idx] = eastl::string8{};
					break;
				}
				case App::PropertyType::String16:
				{
					auto* value_ptr = static_cast<eastl::string16*>(void_value_ptr);
					value_ptr[idx] = eastl::string16{};
					break;
				}
				case App::PropertyType::Key:
				{
					auto* value_ptr = static_cast<ResourceKey*>(void_value_ptr);
					value_ptr[idx] = ResourceKey{};
					break;
				}
				case App::PropertyType::Text:
				{
					auto* value_ptr = static_cast<LocalizedString*>(void_value_ptr);
					value_ptr[idx] = LocalizedString{};
					break;
				}
				case App::PropertyType::Vector2:
				{
					auto* value_ptr = static_cast<Vector2*>(void_value_ptr);
					value_ptr[idx] = Vector2{};
					break;
				}
				case App::PropertyType::Vector3:
				{
					auto* value_ptr = static_cast<Vector3*>(void_value_ptr);
					value_ptr[idx] = Vector3{};
					break;
				}
				case App::PropertyType::ColorRGB:
				{
					auto* value_ptr = static_cast<ColorRGB*>(void_value_ptr);
					value_ptr[idx] = ColorRGB{};
					break;
				}
				case App::PropertyType::Vector4:
				{
					auto* value_ptr = static_cast<Vector4*>(void_value_ptr);
					value_ptr[idx] = Vector4{};
					break;
				}
				case App::PropertyType::ColorRGBA:
				{
					auto* value_ptr = static_cast<ColorRGBA*>(void_value_ptr);
					value_ptr[idx] = ColorRGBA{};
					break;
				}
				case App::PropertyType::Transform:
				{
					auto* value_ptr = static_cast<Transform*>(void_value_ptr);
					value_ptr[idx] = Transform{};
					break;
				}
				case App::PropertyType::BBox:
				{
					auto* value_ptr = static_cast<BoundingBox*>(void_value_ptr);
					value_ptr[idx] = BoundingBox{};
					break;
				}
				default:
				{
					return;
				}
			}
		}
	}

	void* void_value_ptr = property_ext.GetValuePointer();
	switch(property_ext.mnType)
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
		case App::PropertyType::Text:
		{
			auto* value_ptr = static_cast<LocalizedString*>(void_value_ptr);
			value_ptr[idx] = value.as<LocalizedString>();
			break;
		}
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
			const auto type_size = property_ext.GetItemSize();
			memcpy(static_cast<char*>(void_value_ptr) + idx * type_size, value.as<void*>(), type_size);
			break;
		}
		default:
		{
			break;
		}
	}
}

static sol::object PropertyLuaGet(sol::this_state L, App::Property& property, sol::optional<size_t> index)
{
	const sol::state_view s(L);
	const auto idx = index.value_or(1) - 1;
	
	auto& property_ext = GetPropertyExt(property);
	const size_t num_values = property_ext.GetItemCount();

	if (idx > num_values)
	{
		return sol::make_object(L, sol::nil);
	}

	void* void_value_ptr = property_ext.GetValuePointer();
	
	switch(property_ext.mnType)
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
			if (property_ext.GetItemSize() == 8)
			{
				const auto* value_ptr = static_cast<const Extensions::Property::smfx_array_string_8*>(void_value_ptr);
				return sol::make_object(L, sol::string_view(value_ptr->begin, value_ptr->end - value_ptr->begin));
			}
			else
			{
				const auto* value_ptr = static_cast<const eastl::string8*>(void_value_ptr);
				return sol::make_object(L, value_ptr[idx].c_str());
			}
		}
		case App::PropertyType::String16:
		{
			const auto* value_ptr = static_cast<const eastl::string16*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx].c_str());
		}
		case App::PropertyType::Key:
		{
			auto* value_ptr = static_cast<ResourceKey*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Text:
		{
			auto* value_ptr = static_cast<LocalizedString*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Vector2:
		{
			auto* value_ptr = static_cast<Vector2*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Vector3:
		{
			auto* value_ptr = static_cast<Vector3*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::ColorRGB:
		{
			auto* value_ptr = static_cast<ColorRGB*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Vector4:
		{
			auto* value_ptr = static_cast<Vector4*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::ColorRGBA:
		{
			auto* value_ptr = static_cast<ColorRGBA*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::Transform:
		{
			auto* value_ptr = static_cast<Transform*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		case App::PropertyType::BBox:
		{
			auto* value_ptr = static_cast<BoundingBox*>(void_value_ptr);
			return sol::make_object(L, value_ptr[idx]);
		}
		default:
		{
			return sol::make_object(L, sol::nil);
		}
	}
}

static size_t PropertyLuaLength(const App::Property& property)
{
	const auto& property_ext = GetPropertyExt(property);
	return property_ext.IsArray() ? property_ext.GetItemCount() : 1u;
}

AddLuaBinding(Property, sol::state_view s)
{
	s.new_usertype<App::Property>(
		"Property",
		"IsArray", [](const App::Property& property)
		{
			return GetPropertyExt(property).IsArray();
		},
		sol::meta_function::length, PropertyLuaLength,
		"GetItemCount", PropertyLuaLength,
		sol::meta_function::index, PropertyLuaGet,
		"Get", PropertyLuaGet,
		sol::meta_function::new_index, PropertyLuaSet,
		"Set", [](sol::this_state L, App::Property& property, sol::stack_object value)
		{
			PropertyLuaSet(L, property, 1, value);
		},
		"Reserve", PropertyLuaReserve
	);
}

#endif