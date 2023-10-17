#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\Bindings.h>

void LuaAPI::RegisterProperty(sol::state_view& s)
{
	
}

/*
int PropertyReg::IsArray(lua_State* L)
{
	lua_pushboolean(L, mProperty->IsArray());
	return 1;
}

int PropertyReg::GetCount(lua_State* L)
{
	lua_pushinteger(L, mProperty->IsArray() ? mProperty->GetItemCount() : 1);
	return 1;
}

template<typename T>
int PushValue(Extensions::Property* prop, lua_State* L)
{
	const T* value = static_cast<T*>(prop->GetValue());
	const size_t item_count = prop->GetItemCount();
	if (prop->IsArray())
	{
		lua_createtable(L, static_cast<int>(item_count), 0);
		for(size_t i = 0; i < item_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_pushsporetype(L, value[i]);
			lua_settable(L, -3);
		}
		lua_pushinteger(L, item_count);
	}
	else
	{
		lua_pushsporetype(L, *value);
		lua_pushinteger(L, 1);
	}
	return 2;
}

int PropertyReg::GetValueInt32(lua_State* L)
{
	return PushValue<int32_t>(mProperty, L);
}

int PropertyReg::GetValueFloat(lua_State* L)
{
	return PushValue<float>(mProperty, L);
}

int PropertyReg::GetValueBool(lua_State* L)
{
	return PushValue<bool>(mProperty, L);
}

int PropertyReg::GetValueUInt32(lua_State* L)
{
	return PushValue<uint32_t>(mProperty, L);
}

int PropertyReg::GetValueVector2(lua_State* L)
{
	return PushValue<Vector2>(mProperty, L);
}

int PropertyReg::GetValueVector3(lua_State* L)
{
	return PushValue<Vector3>(mProperty, L);
}

int PropertyReg::GetValueVector4(lua_State* L)
{
	return PushValue<Vector4>(mProperty, L);
}

int PropertyReg::GetValueColorRGB(lua_State* L)
{
	return PushValue<ColorRGB>(mProperty, L);
}

int PropertyReg::GetValueColorRGBA(lua_State* L)
{
	return PushValue<ColorRGBA>(mProperty, L);
}

int PropertyReg::GetValueKey(lua_State* L)
{
	return PushValue<ResourceKey>(mProperty, L);
}

int PropertyReg::GetValueTransform(lua_State* L)
{
	return PushValue<Transform>(mProperty, L);
}

int PropertyReg::GetValueText(lua_State* L)
{
	return PushValue<LocalizedString>(mProperty, L);
}

int PropertyReg::GetValueBBox(lua_State* L)
{
	return PushValue<BoundingBox>(mProperty, L);
}

int PropertyReg::GetValueString8(lua_State* L)
{
	return PushValue<eastl::string8>(mProperty, L);
}

int PropertyReg::GetValueString16(lua_State* L)
{
	return PushValue<eastl::string16>(mProperty, L);
}
*/

#endif