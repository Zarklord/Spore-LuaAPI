#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\Types\LuaPropertyArray.h>

using namespace LuaAPI;

size_t GetTypeSize(const App::PropertyType prop_type)
{
	switch(prop_type)
	{
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
			return sizeof(LocalizedString);
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

LuaPropertyArray::LuaPropertyArray(Extensions::Property* prop)
: mType(prop->mnType)
, mArray(static_cast<uint8_t*>(prop->GetValue()))
, mSize(prop->GetItemCount())
, mTypeSize(prop->GetItemSize())
, mReadOnly(true)
{
	assert(!prop->IsArray());
}

LuaPropertyArray::LuaPropertyArray(const App::PropertyType prop_type, size_t array_size)
: mType(prop_type)
, mArray(new uint8_t[array_size])
, mSize(array_size)
, mTypeSize(GetTypeSize(prop_type))
, mReadOnly(false)
{
}

LuaPropertyArray::~LuaPropertyArray()
{
	FreeArray();
	mArray = nullptr;
}

void LuaPropertyArray::FreeArray() const
{
	if (mReadOnly && mArray)
		delete[] mArray;
}

void LuaPropertyArray::SetProperty(App::Property* prop) const
{
	prop->Set(mType, App::Property::kPropertyFlagArray, mArray, mTypeSize, mSize);	
}

sol::object LuaPropertyArray::LuaIndex(sol::this_state L, size_t index) const
{
	if (mReadOnly) return sol::nil;

	const sol::state_view s(L);
	auto * array_val = mArray + (index - 1) * mTypeSize;
	switch(mType)
	{
		case App::PropertyType::Bool:
		{
			return sol::make_object(s, reinterpret_cast<bool&>(*array_val));
		}
		case App::PropertyType::Int32:
		{
			return sol::make_object(s, reinterpret_cast<int32_t&>(*array_val));
		}
		case App::PropertyType::UInt32:
		{
			return sol::make_object(s, reinterpret_cast<uint32_t&>(*array_val));
		}
		case App::PropertyType::Float:
		{
			return sol::make_object(s, reinterpret_cast<float&>(*array_val));
		}
		case App::PropertyType::String8:
		{
			return sol::make_object(s, reinterpret_cast<eastl::string8&>(*array_val));
		}
		case App::PropertyType::String16:
		{
			return sol::make_object(s, reinterpret_cast<eastl::string16&>(*array_val));
		}
		case App::PropertyType::Key:
		{
			return sol::make_object(s, reinterpret_cast<ResourceKey&>(*array_val));
		}
		case App::PropertyType::Text:
		{
			return sol::make_object(s, reinterpret_cast<LocalizedString&>(*array_val));
		}
		case App::PropertyType::Vector2:
		{
			return sol::make_object(s, reinterpret_cast<Vector2&>(*array_val));
		}
		case App::PropertyType::Vector3:
		{
			return sol::make_object(s, reinterpret_cast<Vector3&>(*array_val));
		}
		case App::PropertyType::ColorRGB:
		{
			return sol::make_object(s, reinterpret_cast<ColorRGB&>(*array_val));
		}
		case App::PropertyType::Vector4:
		{
			return sol::make_object(s, reinterpret_cast<Vector4&>(*array_val));
		}
		case App::PropertyType::ColorRGBA:
		{
			return sol::make_object(s, reinterpret_cast<ColorRGBA&>(*array_val));
		}
		case App::PropertyType::Transform:
		{
			return sol::make_object(s, reinterpret_cast<Transform&>(*array_val));
		}
		case App::PropertyType::BBox:
		{
			return sol::make_object(s, reinterpret_cast<BoundingBox&>(*array_val));
		}
		default:
		{
			assert(false && "invalid App::PropertyType");
		}
	}
	return sol::nil;
}

void LuaPropertyArray::LuaNewIndex(size_t index, sol::stack_object value) const
{
	if (mReadOnly) return;

	auto * array_val = mArray + (index - 1) * mTypeSize;
	if (mType == App::PropertyType::String8)
	{
		auto new_str = value.as<eastl::string8>();
		const auto old_str = reinterpret_cast<eastl::string8*>(array_val);
		old_str->swap(new_str);
	}
	else if (mType == App::PropertyType::String16)
	{
		auto new_str = value.as<eastl::string16>();
		const auto old_str = reinterpret_cast<eastl::string16*>(array_val);
		old_str->swap(new_str);
	}
	else
	{
		memcpy(array_val, value.as<void*>(), mTypeSize);
	}
}

#endif