#include <pch.h>
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore\DefaultIncludes.h>

class PropManagerReg
{
public:
	PropManagerReg()
	: mPropManager(App::IPropManager::Get())
	{
	}

	int GetPropertyList(lua_State* L)
	{
		const auto group = lua_checkfnvhash(L, 1);
		const auto instance = lua_checkfnvhash(L, 2);

		PropertyListPtr property_list;
		if (!mPropManager->GetPropertyList(instance, group, property_list))
		{
			lua_pushnil(L);
		}
		else
		{
			//Lunar<PropertyListReg>::push(L, new PropertyListReg(property_list.get()));
		}
		return 1;
	}

public:
	static inline constexpr char LuaClassName[] = "PropManager";
	static LunarRegType LuaMethods[];
private:
	IPropManagerPtr mPropManager;
};

LunarRegType PropManagerReg::LuaMethods[] = {
	LUNAR_MEMBER_FUNCTION(PropManagerReg, GetPropertyList),
};

void RegisterPropManagerReg(lua_State* L)
{
	Lunar<PropManagerReg>::Register(L);

    Lunar<PropManagerReg>::push(L, new PropManagerReg());
    lua_setglobal(L, "PropManager");
}


#endif