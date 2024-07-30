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
		"instance_id", sol::property([](const ResourceKey& key){ return key.instanceID; }, [](ResourceKey& key, const LuaFNVHash instanceID){ key.instanceID = instanceID; }),
		"group_id", sol::property([](const ResourceKey& key){ return key.groupID; }, [](ResourceKey& key, const LuaFNVHash groupID){ key.groupID = groupID; }),
		"type_id", sol::property([](const ResourceKey& key){ return key.typeID; }, [](ResourceKey& key, const LuaFNVHash typeID){ key.typeID = typeID; }),
		sol::meta_function::to_string, [](const ResourceKey& key) { return eastl::string().sprintf("ResourceKey 0x%X!0x%X.0x%X", key.groupID, key.instanceID, key.typeID); }
	);


	s.new_usertype<App::Property::TextProperty>(
		"TextProperty",
		sol::no_constructor,
		"table_id", sol::readonly(&App::Property::TextProperty::tableID),
		"instance_id", sol::readonly(&App::Property::TextProperty::instanceID),
		"buffer", sol::readonly(&App::Property::TextProperty::buffer),
		sol::meta_function::to_string, [](const App::Property::TextProperty& text)
		{
			return string().sprintf("App::Property::TextProperty (0x%X!0x%X) \"%ls\"", text.tableID, text.instanceID, text.buffer);
		},
		sol::meta_function::equal_to, [](const App::Property::TextProperty& a, const App::Property::TextProperty& b)
		{
			return a.tableID == b.tableID && a.instanceID == b.instanceID &&
				sol::u16string_view(a.buffer) == sol::u16string_view(b.buffer);
		}
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
		),
		sol::meta_function::to_string, [](const LocalizedString& localized_string)
		{
			return string().sprintf("LocalizedString \"%ls\"", localized_string.GetText());
		},
		sol::meta_function::equal_to, [](const LocalizedString& a, const LocalizedString& b)
		{
			return a.GetText() == b.GetText();
		}
	);

	s.new_usertype<Vector2>(
		"Vector2",
		sol::call_constructor, sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y,
		sol::meta_function::to_string, [](const Vector2& vec) { return string().sprintf("Vector2 (%2.2f, %2.2f)", vec.x, vec.y); }
	);

	s.new_usertype<Vector3>(
		"Vector3",
		sol::call_constructor, sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z,
		sol::meta_function::to_string, [](const Vector3& vec) { return string().sprintf("Vector3 (%2.2f, %2.2f, %2.2f)", vec.x, vec.y, vec.z); }
	);

	s.new_usertype<Vector4>(
		"Vector4",
		sol::call_constructor, sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w,
		sol::meta_function::to_string, [](const Vector4& vec) { return string().sprintf("Vector4 (%2.2f, %2.2f, %2.2f, %2.2f)", vec.x, vec.y, vec.z, vec.w); }
	);

	s.new_usertype<ColorRGB>(
		"ColorRGB",
		sol::call_constructor, sol::constructors<ColorRGB(), ColorRGB(float, float, float)>(),
		"r", &ColorRGB::r,
		"g", &ColorRGB::g,
		"b", &ColorRGB::b,
		sol::meta_function::to_string, [](const ColorRGB& color) { return string().sprintf("ColorRGB (%2.2f, %2.2f, %2.2f)", color.r, color.g, color.b); }
	);

	s.new_usertype<ColorRGBA>(
		"ColorRGBA",
		sol::call_constructor, sol::constructors<ColorRGBA(), ColorRGBA(float, float, float, float)>(),
		"r", &ColorRGBA::r,
		"g", &ColorRGBA::g,
		"b", &ColorRGBA::b,
		"a", &ColorRGBA::a,
		sol::meta_function::to_string, [](const ColorRGBA& color) { return string().sprintf("ColorRGBA (%2.2f, %2.2f, %2.2f, %2.2f)", color.r, color.g, color.b, color.a); }
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
		},
		sol::meta_function::to_string, [](const Matrix3& mat)
		{
			return string().sprintf("Matrix3 (%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f)",
				mat.m[1][1], mat.m[1][2], mat.m[1][3],
				mat.m[2][1], mat.m[2][2], mat.m[2][3],
				mat.m[3][1], mat.m[3][2], mat.m[3][3]);
		},
		sol::meta_function::equal_to, [](const Matrix3& a, const Matrix3& b)
		{
			return memcmp(a.m, b.m, sizeof(a.m)) == 0;
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
		),
		sol::meta_function::to_string, [](const Transform& transform)
		{
			const auto vec = transform.GetOffset();
			const auto scale = transform.GetScale();
			const auto rotation = transform.GetRotation();
			return string().sprintf("Transform Offset(%2.2f, %2.2f, %2.2f) Scale(%2.2f) Rotation(%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f)",
				vec.x, vec.y, vec.z,
				scale,
				rotation.m[1][1], rotation.m[1][2], rotation.m[1][3],
				rotation.m[2][1], rotation.m[2][2], rotation.m[2][3],
				rotation.m[3][1], rotation.m[3][2], rotation.m[3][3]);
		},
		sol::meta_function::equal_to, [](const Transform& a, const Transform& b)
		{
			return a.GetOffset() == b.GetOffset() && a.GetScale() == b.GetScale() &&
				memcmp(a.GetRotation().m, b.GetRotation().m, sizeof(a.GetRotation().m)) == 0;
		}
	);

	s.new_usertype<BoundingBox>(
		"BoundingBox",
		sol::call_constructor, sol::constructors<BoundingBox(), BoundingBox(const Vector3&, const Vector3&)>(),
		"lower", &BoundingBox::lower,
		"upper", &BoundingBox::upper,
		sol::meta_function::to_string, [](const BoundingBox& bbox)
		{
			return string().sprintf("BoundingBox (%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f)",
				bbox.upper.x, bbox.upper.y, bbox.upper.z,
				bbox.lower.x, bbox.lower.y, bbox.lower.z);
		},
		sol::meta_function::equal_to, [](const BoundingBox& a, const BoundingBox& b)
		{
			return a.lower == b.lower && a.upper == b.upper;
		}
	);	
}

#endif