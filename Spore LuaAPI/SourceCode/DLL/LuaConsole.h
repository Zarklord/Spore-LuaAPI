#pragma once

#include <LuaSpore\DefaultIncludes.h>

namespace LuaConsole {
	void AttachDetours();
	void LuaInitialize(sol::state_view& s);
	void LuaDispose(sol::state_view& s);
}
