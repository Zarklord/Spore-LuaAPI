#pragma once

#include <LuaSpore\LuaInternal.h>
#include <LuaSpore\DefaultIncludes.h>

namespace LuaAPI
{
	class LuaBinding
	{
	public:
		LuaBinding(const LuaBinding&) = delete;
	    LuaBinding& operator=(const LuaBinding&) = delete;
		LuaBinding(LuaBinding&&) = delete;
	    LuaBinding& operator=(LuaBinding&&) = delete;
	protected:
		LUAAPI LuaBinding(LuaFunction f);
		virtual ~LuaBinding() = default;
	LUA_INTERNALPUBLIC:
		static void ExecuteLuaBindings(lua_State* L);
	private:
		static std::vector<LuaFunction> sLuaBindings;
	};

	template <typename T>
	class LuaBindingInstance;

	template <typename T>
	class LuaBindingImpl final : public LuaBinding
	{
	public:
		LuaBindingImpl(const LuaBindingImpl&) = delete;
	    LuaBindingImpl& operator=(const LuaBindingImpl&) = delete;
		LuaBindingImpl(LuaBindingImpl&&) = delete;
	    LuaBindingImpl& operator=(LuaBindingImpl&&) = delete;
	private:
		using LuaBindingInstanceClass = LuaBindingInstance<T>;
		friend class LuaBindingInstanceClass;

		LuaBindingImpl() : LuaBinding(LuaBindingInstanceClass::InternalBindingFunction) {}
		~LuaBindingImpl() override = default;
	};

	template <typename T>
	class LuaBindingInstance
	{
	private:
		using LuaBindingImplClass = LuaBindingImpl<T>;
		friend class LuaBindingImplClass;

		static LuaBindingImplClass s_lua_binding_inst;

		static void InternalBindingFunction(lua_State* L)
		{
			T::BindingFunction(L);
		}
	};

	template <typename T>
	inline LuaBindingImpl<T> LuaBindingInstance<T>::s_lua_binding_inst;
}

#define _LuaBindingConcatImpl(x, y) x##y
#define _LuaBindingConcat(x, y) _LuaBindingConcatImpl(x, y)
#define _AddLuaBindingImpl(class_name, binding_arg)							 \
	namespace LuaBindings  													 \
	{																		 \
		namespace  															 \
		{																	 \
			class class_name : public LuaAPI::LuaBindingInstance<class_name> \
			{																 \
				static void BindingFunction(binding_arg);					 \
			};																 \
		}																	 \
	}																		 \
	void LuaBindings::class_name::BindingFunction(binding_arg)
#define AddLuaBinding(binding_arg) _AddLuaBindingImpl(_LuaBindingConcat(LuaBindingCallback, __COUNTER__), binding_arg)