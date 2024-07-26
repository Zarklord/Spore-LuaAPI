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

#include "pch.h"

#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSporeCallbacks.h>

OnLuaInit(sol::state_view s, bool is_main_state)
{
	s.new_usertype<ResourceKey>(
		"ResourceKey",
		sol::call_constructor, sol::initializers(
			[](ResourceKey* memory, const LuaFNVHash& groupID, const LuaFNVHash& instanceID, const LuaFNVHash& typeID)
			{
				new(memory) ResourceKey(instanceID, typeID, groupID);
			}
		),
		"instance_id", &ResourceKey::instanceID,
		"group_id", &ResourceKey::groupID,
		"type_id", &ResourceKey::typeID
	);

	s.new_usertype<App::Property::TextProperty>(
		"TextProperty",
		sol::no_constructor,
		"table_id", sol::readonly(&App::Property::TextProperty::tableID),
		"instance_id", sol::readonly(&App::Property::TextProperty::instanceID),
		"buffer", sol::readonly(&App::Property::TextProperty::buffer)
	);

	s.new_usertype<LocalizedString>(
		"LocalizedString",
		sol::call_constructor, sol::initializers(
			[](LocalizedString* memory, const LuaFNVHash& tableID, const LuaFNVHash& instanceID, sol::optional<const char16_t*> text)
			{
				new(memory) LocalizedString(tableID, instanceID, text.value_or(nullptr));
			},
			[](LocalizedString* memory, const App::Property::TextProperty& text)
			{
				new(memory) LocalizedString(text.tableID, text.instanceID);
			}
		),
		"GetText", &LocalizedString::GetText,
		"SetText", sol::overload(
			[](LocalizedString& str, const LuaFNVHash& tableID, const LuaFNVHash& instanceID, sol::optional<const char16_t*> text)
			{
				str.SetText(tableID, instanceID, text.value_or(nullptr));
			},
			[](LocalizedString& str, const App::Property::TextProperty& text)
			{
				str.SetText(text.tableID, text.instanceID);
			}
		)
	);

	s.new_usertype<Vector2>(
		"Vector2",
		sol::call_constructor, sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y
	);

	s.new_usertype<Vector3>(
		"Vector3",
		sol::call_constructor, sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z
	);

	s.new_usertype<Vector4>(
		"Vector4",
		sol::call_constructor, sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w
	);

	s.new_usertype<ColorRGB>(
		"ColorRGB",
		sol::call_constructor, sol::constructors<ColorRGB(), ColorRGB(float, float, float)>(),
		"r", &ColorRGB::r,
		"g", &ColorRGB::g,
		"b", &ColorRGB::b
	);

	s.new_usertype<ColorRGBA>(
		"ColorRGBA",
		sol::call_constructor, sol::constructors<ColorRGBA(), ColorRGBA(float, float, float, float)>(),
		"r", &ColorRGBA::r,
		"g", &ColorRGBA::g,
		"b", &ColorRGBA::b,
		"a", &ColorRGBA::a
	);

	s.new_usertype<Matrix3>(
		"Matrix3",
		sol::call_constructor, sol::constructors<Matrix3()>(),
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
		sol::call_constructor, sol::constructors<Transform()>(),
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
		sol::call_constructor, sol::constructors<BoundingBox(), BoundingBox(const Vector3&, const Vector3&)>(),
		"lower", &BoundingBox::lower,
		"upper", &BoundingBox::upper
	);	
}

#endif