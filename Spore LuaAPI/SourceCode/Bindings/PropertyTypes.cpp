#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\Bindings.h>

void LuaAPI::RegisterPropertyTypes(sol::state_view& s)
{
	s.new_usertype<ResourceKey>(
		"ResourceKey",
		sol::call_constructor,
		sol::factories([](const LuaFNVHash& groupID, const LuaFNVHash& instanceID, const LuaFNVHash& typeID)
		{
			return ResourceKey(instanceID, typeID, groupID);
		}),
		"instance_id", &ResourceKey::instanceID,
		"group_id", &ResourceKey::groupID,
		"type_id", &ResourceKey::typeID
	);

	s.new_usertype<LocalizedString>(
		"LocalizedString",
		sol::call_constructor,
		sol::factories(
			[](const LuaFNVHash& tableID, const LuaFNVHash& instanceID)
			{
				return LocalizedString(tableID, instanceID);
			}, 
			[](const LuaFNVHash& tableID, const LuaFNVHash& instanceID, const char16_t* text)
			{
				return LocalizedString(tableID, instanceID, text);
			}
		),
		"GetText", &LocalizedString::GetText,
		"SetText", sol::overload(
			[](LocalizedString& str, const LuaFNVHash& tableID, const LuaFNVHash& instanceID)
			{
				str.SetText(tableID, instanceID);
			},
			[](LocalizedString& str, const LuaFNVHash& tableID, const LuaFNVHash& instanceID, const char16_t* text)
			{
				str.SetText(tableID, instanceID, text);
			}
		)
	);

	s.new_usertype<Vector2>(
		"Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y
	);

	s.new_usertype<Vector3>(
		"Vector3",
		sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z
	);

	s.new_usertype<Vector4>(
		"Vector4",
		sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w
	);

	s.new_usertype<ColorRGB>(
		"ColorRGB",
		sol::constructors<ColorRGB(), ColorRGB(float, float, float)>(),
		"r", &ColorRGB::r,
		"g", &ColorRGB::g,
		"b", &ColorRGB::b
	);

	s.new_usertype<ColorRGBA>(
		"ColorRGBA",
		sol::constructors<ColorRGBA(), ColorRGBA(float, float, float, float)>(),
		"r", &ColorRGBA::r,
		"g", &ColorRGBA::g,
		"b", &ColorRGBA::b,
		"a", &ColorRGBA::a
	);

	s.new_usertype<Matrix3>(
		"Matrix3",
		sol::constructors<Matrix3()>(),

		"ToEuler", &Matrix3::ToEuler,
		"Get", [](const Matrix3& mat, int row, int column)
		{
			return mat.m[row][column];
		},
		"Set", [](Matrix3& mat, int row, int column, float value)
		{
			mat.m[row][column] = value;
		}
	);

	s.new_usertype<Transform>(
		"Transform",
		sol::constructors<Transform()>(),
		
		"GetOffset", &Transform::GetOffset,
		"SetOffset", sol::overload(
			[](Transform& transform, const Vector3& value)
			{
				transform.SetOffset(value);
			},
			[](Transform& transform, float x, float y, float z)
			{
				transform.SetOffset(x, y ,z);
			}
		),
		
		"GetScale", &Transform::GetScale,
		"SetScale", [](Transform& transform, float value)
		{
			transform.SetScale(value);
		},
		
		"GetRotation", &Transform::GetRotation,
		"SetRotation", sol::overload(
			[](Transform& transform, const Matrix3& value)
			{
				transform.SetRotation(value);
			},
			[](Transform& transform, const Vector3& euler)
			{
				transform.SetRotation(euler);
			}
		)
	);

	s.new_usertype<BoundingBox>(
		"BoundingBox",
		sol::constructors<BoundingBox(), BoundingBox(const Vector3&, const Vector3&)>(),
		"lower", &BoundingBox::lower,
		"upper", &BoundingBox::upper
	);
}

#endif