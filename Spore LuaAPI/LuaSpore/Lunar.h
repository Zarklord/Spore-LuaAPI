#pragma once

#include <LuaSpore\LuaInternal.h>

typedef struct { const char *name; lua_CFunction func; } LunarRegType;

template <typename T>
class Lunar
{
public:
    static constexpr bool LuaConstructible = std::is_constructible_v<T, lua_State*>;

    static void Register(lua_State* L) {
        lua_newtable(L);
        const int methods = lua_gettop(L);

        luaL_newmetatable(L, T::LuaClassName);
        const int metatable = lua_gettop(L);

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushvalue(L, methods);
        lua_setglobal(L, T::LuaClassName);

        lua_pushvalue(L, methods);
        set(L, metatable, "__index");

        lua_pushcfunction(L, tostring_T);
        set(L, metatable, "__tostring");

        lua_pushcfunction(L, gc_T);
        set(L, metatable, "__gc");

        lua_newtable(L); //mt for method table

        if constexpr(LuaConstructible)
        {
		    lua_pushcfunction(L, new_T);
		    lua_pushvalue(L, -1);
		    set(L, methods, "new");
		    set(L, -3, "__call");
        }

        lua_setmetatable(L, methods);

        //fill method table with methods from class T
        for (const LunarRegType& method : T::LuaMethods)
        {
            lua_pushstring(L, method.name);
            lua_pushcfunction(L, method.func);
            lua_settable(L, methods);
        }

        lua_pop(L, 2);  // drop metatable and method table
    }

    // call named lua method from userdata method table
    static int call(lua_State* L, const char *method, int nargs=0, int nresults=LUA_MULTRET, int errfunc=0)
    {
        const int base = lua_gettop(L) - nargs; // userdata index
        if (!luaL_checkudata(L, base, T::LuaClassName))
        {
            lua_settop(L, base-1); // drop userdata and args
            lua_pushfstring(L, "not a valid %s userdata", T::LuaClassName);
            return -1;
        }

        lua_pushstring(L, method); // method name
        lua_gettable(L, base); // get method from userdata
        if (lua_isnil(L, -1)) // no method?
        {
            lua_settop(L, base-1); // drop userdata and args
            lua_pushfstring(L, "%s missing method '%s'", T::LuaClassName, method);
            return -1;
        }
        lua_insert(L, base); // put method under userdata, args

        const int status = lua_pcall(L, 1+nargs, nresults, errfunc); // call method
        if (status)
        {
            const char *msg = lua_tostring(L, -1);
            if (msg == nullptr) msg = "(error with no message)";
            lua_pushfstring(L, "%s:%s status = %d\n%s", T::LuaClassName, method, status, msg);
            lua_remove(L, base); // remove old message
            return -1;
        }
        return lua_gettop(L) - base + 1; // number of results
    }

    // push onto the Lua stack a userdata containing a pointer to T object
    static T* push(lua_State* L, T* newT) 
    {
        luaL_getmetatable(L, T::LuaClassName);  // lookup metatable in Lua registry
        if (lua_isnil(L, -1)) luaL_error(L, "%s missing metatable", T::LuaClassName);
        const int mt = lua_gettop(L);
        *static_cast<T**>(lua_newuserdatauv(L, sizeof(T*), 1)) = newT; // create new userdata
        lua_pushvalue(L, mt);
        lua_setmetatable(L, -2); //set the metatable of the metatable of the userdata
        lua_remove(L, -2); // remove metatable
        return newT;
    }

    // get userdata from Lua stack and return pointer to T object
    static T* check(lua_State* L, int narg) {
        T** ud = static_cast<T**>(luaL_testudata(L, narg, T::LuaClassName));
        if(luai_unlikely(!ud))
        {
            luaL_typeerror(L, narg, T::LuaClassName);
            return nullptr;
        }
        return *ud;
    }

private:
    Lunar() = default;  // hide default constructor

    template <typename = std::enable_if<LuaConstructible>>
	static int new_T(lua_State *L) {
		lua_remove(L, 1);
		T *obj = new T(L);
		push(L, obj, true);
		return 1;
	}

    // garbage collection metamethod
    static int gc_T(lua_State* L)
    {
        delete *static_cast<T**>(lua_touserdata(L, 1));
        return 0;
    }

    static int tostring_T(lua_State* L)
    {
        char buff[32];
        T* obj = *static_cast<T**>(lua_touserdata(L, 1));
        snprintf(buff, 32, "%p", static_cast<void*>(obj));
        lua_pushfstring(L, "%s (%s)", T::LuaClassName, buff);

        return 1;
    }

    static void set(lua_State* L, int table_index, const char* key)
    {
        lua_pushstring(L, key);
        lua_insert(L, -2);  // swap value and key
        lua_settable(L, table_index);
    }
};

#define LUNAR_MEMBER_FUNCTION(Class, Name) {#Name, [](lua_State *L) -> int { \
    /*stack has userdata, followed by method args*/ \
    Class *obj = Lunar<Class>::check(L, 1); /*get the obj */ \
    lua_remove(L, 1); /*remove obj so args start at index 1*/ \
    return obj->Name(L); /*call member function*/\
}}

#define LUNAR_STATIC_FUNCTION(Fn, Name) {#Name, &Fn}
