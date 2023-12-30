#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\SporeDetours.h>

using namespace LuaAPI;

std::vector<AttachDetoursFunction> SporeDetours::sSporeDetours;

SporeDetours::SporeDetours(AttachDetoursFunction f)
{
	sSporeDetours.push_back(f);	
}

void SporeDetours::AttachSporeDetours()
{
	for (const auto function : sSporeDetours)
		function();
}

#endif