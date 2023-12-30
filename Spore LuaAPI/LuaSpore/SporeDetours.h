#pragma once

#include <LuaSpore\LuaInternal.h>
#include <LuaSpore\DefaultIncludes.h>

namespace LuaAPI
{
	typedef void(*AttachDetoursFunction)();
	class SporeDetours
	{
	public:
		SporeDetours(const SporeDetours&) = delete;
	    SporeDetours& operator=(const SporeDetours&) = delete;
		SporeDetours(SporeDetours&&) = delete;
	    SporeDetours& operator=(SporeDetours&&) = delete;
	protected:
		LUAAPI SporeDetours(AttachDetoursFunction f);
		virtual ~SporeDetours() = default;
	LUA_INTERNALPUBLIC:
		static void AttachSporeDetours();
	private:
		static std::vector<AttachDetoursFunction> sSporeDetours;
	};

	template <typename T>
	class SporeDetoursInstance;

	template <typename T>
	class SporeDetoursImpl final : public SporeDetours
	{
	public:
		SporeDetoursImpl(const SporeDetoursImpl&) = delete;
	    SporeDetoursImpl& operator=(const SporeDetoursImpl&) = delete;
		SporeDetoursImpl(SporeDetoursImpl&&) = delete;
	    SporeDetoursImpl& operator=(SporeDetoursImpl&&) = delete;
	private:
		using SporeDetoursInstanceClass = SporeDetoursInstance<T>;
		friend class SporeDetoursInstanceClass;

		SporeDetoursImpl() : SporeDetours(SporeDetoursInstanceClass::InternalAttachDetoursFunction) {}
		~SporeDetoursImpl() override = default;
	};

	template <typename T>
	class SporeDetoursInstance
	{
	private:
		using SporeDetoursImplClass = SporeDetoursImpl<T>;
		friend class SporeDetoursImplClass;

		static SporeDetoursImplClass s_lua_detours_inst;

		static void InternalAttachDetoursFunction()
		{
			T::AttachDetoursFunction();
		}
	};

	template <typename T>
	inline SporeDetoursImpl<T> SporeDetoursInstance<T>::s_lua_detours_inst;
}

#define _SporeDetoursConcatImpl(x, y) x##y
#define _SporeDetoursConcat(x, y) _SporeDetoursConcatImpl(x, y)
#define _AddSporeDetoursImpl(class_name)											\
	namespace SporeDetours  														\
	{																				\
		namespace  																	\
		{																			\
			class class_name : public LuaAPI::SporeDetoursInstance<class_name>		\
			{																		\
				static void AttachDetoursFunction();								\
			};																		\
		}																			\
	}																				\
	void SporeDetours::class_name::AttachDetoursFunction()
#define AddSporeDetours() _AddSporeDetoursImpl(_LuaBindingConcat(SporeAttachDetoursCallback, __COUNTER__))