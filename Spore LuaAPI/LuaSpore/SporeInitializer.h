#pragma once

#include <LuaSpore\LuaInternal.h>
#include <LuaSpore\DefaultIncludes.h>

namespace LuaAPI
{
	typedef void(*InitializeFunction)();
	class SporeInitializer
	{
	public:
		SporeInitializer(const SporeInitializer&) = delete;
	    SporeInitializer& operator=(const SporeInitializer&) = delete;
		SporeInitializer(SporeInitializer&&) = delete;
	    SporeInitializer& operator=(SporeInitializer&&) = delete;
	protected:
		LUAAPI SporeInitializer(InitializeFunction f);
		virtual ~SporeInitializer() = default;
	LUA_INTERNALPUBLIC:
		static void ExecuteSporeInitializers();
	private:
		static std::vector<InitializeFunction> sSporeInitializers;
	};

	template <typename T>
	class SporeInitializerInstance;

	template <typename T>
	class SporeInitializerImpl final : public SporeInitializer
	{
	public:
		SporeInitializerImpl(const SporeInitializerImpl&) = delete;
	    SporeInitializerImpl& operator=(const SporeInitializerImpl&) = delete;
		SporeInitializerImpl(SporeInitializerImpl&&) = delete;
	    SporeInitializerImpl& operator=(SporeInitializerImpl&&) = delete;
	private:
		using SporeInitializerInstanceClass = SporeInitializerInstance<T>;
		friend class SporeInitializerInstanceClass;

		SporeInitializerImpl() : SporeInitializer(SporeInitializerInstanceClass::InternalInitializerFunction) {}
		~SporeInitializerImpl() override = default;
	};

	template <typename T>
	class SporeInitializerInstance
	{
	private:
		using SporeInitializerImplClass = SporeInitializerImpl<T>;
		friend class SporeInitializerImplClass;

		static SporeInitializerImplClass s_lua_initializer_inst;

		static void InternalInitializeFunction()
		{
			T::InitializeFunction();
		}
	};

	template <typename T>
	inline SporeInitializerImpl<T> SporeInitializerInstance<T>::s_lua_initializer_inst;
}

#define _SporeInitializerConcatImpl(x, y) x##y
#define _SporeInitializerConcat(x, y) _SporeInitializerConcatImpl(x, y)
#define _AddSporeInitializerImpl(class_name)											\
	namespace SporeInitializer  														\
	{																				\
		namespace  																	\
		{																			\
			class class_name : public LuaAPI::SporeInitializerInstance<class_name>	\
			{																		\
				static void InitializeFunction();									\
			};																		\
		}																			\
	}																				\
	void SporeInitializer::class_name::InitializeFunction()
#define AddSporeInitializer() _AddSporeInitializerImpl(_LuaBindingConcat(SporeInitializerCallback, __COUNTER__))