#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/SporeInitializer.h>

using namespace LuaAPI;

std::vector<InitializeFunction> SporeInitializer::sSporeInitializers;

SporeInitializer::SporeInitializer(InitializeFunction f)
{
	sSporeInitializers.push_back(f);	
}

void SporeInitializer::ExecuteSporeInitializers()
{
	for (const auto function : sSporeInitializers)
		function();
}

#endif