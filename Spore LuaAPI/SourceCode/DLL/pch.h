#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Spore\BasicIncludes.h>
#include <Spore\App\cCheatManager.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}