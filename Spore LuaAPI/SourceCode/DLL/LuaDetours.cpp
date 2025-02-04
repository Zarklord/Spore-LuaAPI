#include "pch.h"

#include "Spore/App/cPropManager.h"
#ifdef LUAAPI_DLL_EXPORT

#include <LuaSpore/LuaSpore.h>
#include <LuaSpore/LuaSporeCallbacks.h>

#include <asmjit/asmjit.h>
#include <detours.h>

using namespace asmjit;

namespace LuaCallingConvention
{
	enum Type : uint8_t
	{
		UNKNOWN		 = 0,
		__CDECL		 = 1,
		__CLRCALL	 = 2,
		__STDCALL	 = 3,
		__FASTCALL	 = 4,
		__THISCALL	 = 5,
		__VECTORCALL = 6,
	};
}

namespace
{
	template <class X, class Y>
	union horrible_union{
		X out;
		Y in;
	};

	template <class X, class Y>
	inline X horrible_cast(const Y input){
		horrible_union<X, Y> u;
		// Cause a compile-time error if in, out and u are not the same size.
		// If the compile fails here, it means the compiler has peculiar
		// unions which would prevent the cast from working.
		typedef int ERROR_CantUseHorrible_cast[sizeof(Y)==sizeof(u) 
			&& sizeof(Y)==sizeof(X) ? 1 : -1];
		u.in = input;
		return u.out;
	}
}

struct LuaDetourFunctionInfo
{
	uintptr_t disk_address{};
	uintptr_t address{};
	//LuaCallingConvention::Type calling_convention;

	void* GetFunctionPointer() const
	{
		return reinterpret_cast<void*>(Address(ModAPI::ChooseAddress(disk_address, address))); //NOLINT
	}
};

class ModAPILogger : public Logger {
public:
	ASMJIT_NONCOPYABLE(ModAPILogger)

	ModAPILogger() noexcept
	{
		addFlags(FormatFlags::kMachineCode | FormatFlags::kHexImms | FormatFlags::kHexOffsets);
	}
	~ModAPILogger() noexcept override = default;

	Error _log(const char* data, size_t size = SIZE_MAX) noexcept override
	{
		//ModAPI::Log(data);
		return kErrorOk;
	}
};

static ModAPILogger mod_api_logger;

class LuaDetour
{
private:
	static void SaveASMRegisters(x86::Assembler& a)
	{
		a.push(x86::ebp);
		a.mov(x86::ebp, x86::esp);

		a.push(x86::eax);
		a.push(x86::ecx);
		a.push(x86::edx);
	}
	static void LoadASMRegisters(x86::Assembler& a)
	{
		a.pop(x86::edx);
		a.pop(x86::ecx);
		a.pop(x86::eax);

		a.mov(x86::esp, x86::ebp);
		a.pop(x86::ebp);
	}

	class LuaDetourCallInstance
	{
	public:
		LuaDetourCallInstance(const sol::function& fn, uintptr_t return_address, uintptr_t original_function, uintptr_t cleanup_and_ret_function, uintptr_t cleanup_and_jmp_function)
		{
			mLuaThread = sol::thread::create(fn.lua_state());
			mCallInstance = sol::coroutine(mLuaThread.state(), fn);

			const auto imm_this = Imm(reinterpret_cast<uintptr_t>(this));

			CodeHolder code;
			code.init(mRuntime.environment(), mRuntime.cpuFeatures());
			code.setLogger(&mod_api_logger);
			x86::Assembler a(&code);

			{
				//const bool yielded = this->ExecuteCoroutine();
				a.mov(x86::ecx, imm_this);
				a.call(Imm(horrible_cast<uintptr_t>(&LuaDetourCallInstance::ExecuteCoroutine)));
							
				Label post_execute = a.newLabel();

				//if (!yielded)
				a.test(x86::eax, Imm(1));
				a.jnz(post_execute);

				//jump to cleanup_and_jmp_function address
				a.mov(x86::eax, Imm(this));
				a.jmp(Imm(cleanup_and_jmp_function));

				a.bind(post_execute);
			}

			{
				LoadASMRegisters(a);
			
				//now that we have captured the game return address, remove it from the stack
				a.add(x86::esp, 4);
			
				//call the original function
				a.call(Imm(original_function));
				
				//and undo the removal from the stack
				a.sub(x86::esp, 4);
			
				SaveASMRegisters(a);

				//restore the original return address pointer
				a.mov(x86::dword_ptr_rel(4, x86::ebp), Imm(return_address));
			}
			
			{
				//this->ExecuteCoroutine();
				a.mov(x86::ecx, imm_this);
				a.call(Imm(horrible_cast<uintptr_t>(&LuaDetourCallInstance::ExecuteCoroutine)));
			}
			
			{
				a.mov(x86::eax, Imm(this));
				a.jmp(Imm(cleanup_and_ret_function));
			}

			mRuntime._add(&mJitFunction, &code);
		}

		~LuaDetourCallInstance()
		{
			mRuntime._release(mJitFunction);
			mJitFunction = nullptr;
		}
		
		LuaDetourCallInstance(const LuaDetourCallInstance&) = delete;
		LuaDetourCallInstance& operator=(const LuaDetourCallInstance&) = delete;
		LuaDetourCallInstance(LuaDetourCallInstance&&) = delete;
		LuaDetourCallInstance& operator=(LuaDetourCallInstance&&) = delete;

		uintptr_t GetJitFunctionAddress() const
		{
			return reinterpret_cast<uintptr_t>(mJitFunction);
		}
	private:
		bool ExecuteCoroutine()
		{
			sol::state_view s = mLuaThread.lua_state();

			GetLuaSpore().LockThreadState(s);
			mCallInstance(mLuaThread.state()["coroutine"]["yield"]);
			GetLuaSpore().UnlockThreadState(s);

			return mCallInstance.runnable();
		}

		sol::thread mLuaThread;
		sol::coroutine mCallInstance;
		void* mJitFunction = nullptr;

		JitRuntime mRuntime;
	};

public:
	LuaDetour(LuaDetourFunctionInfo function_info, sol::function fn)
	: mFunctionInfo(function_info)
	, mOriginalFunction(mFunctionInfo.GetFunctionPointer())
	{
		GetLuaSpore().CopyFunctionToAllStates(fn, mLuaFunction);

		const auto imm_this = Imm(reinterpret_cast<uintptr_t>(this));

		{
			CodeHolder code;
			code.init(mRuntime.environment(), mRuntime.cpuFeatures());
			code.setLogger(&mod_api_logger);
			x86::Assembler a(&code);

			SaveASMRegisters(a);

			//grab the return address pointer from right behind ebp
			a.mov(x86::ecx, x86::dword_ptr_rel(4, x86::ebp));
			a.push(x86::ecx);

			a.mov(x86::ecx, imm_this);
			a.call(Imm(horrible_cast<uintptr_t>(&LuaDetour::NewLuaDetourCallInstance)));

			a.jmp(x86::eax);
			
			mRuntime._add(&mDetourFunction, &code);
		}

		{
			CodeHolder code;
			code.init(mRuntime.environment(), mRuntime.cpuFeatures());
			code.setLogger(&mod_api_logger);
			x86::Assembler a(&code);

			a.push(x86::eax); //LuaDetourCallInstance will jump to this instruction with eax set to the this*
				
			a.mov(x86::ecx, imm_this);
			a.call(Imm(horrible_cast<uintptr_t>(&LuaDetour::FreeLuaDetourCallInstance)));

			LoadASMRegisters(a);
			a.ret();

			mRuntime._add(&mCleanupAndRetFunction, &code);
		}

		{
			CodeHolder code;
			code.init(mRuntime.environment(), mRuntime.cpuFeatures());
			code.setLogger(&mod_api_logger);
			x86::Assembler a(&code);

			a.push(x86::eax); //LuaDetourCallInstance will jump to this instruction with eax set to the this*
				
			a.mov(x86::ecx, imm_this);
			a.call(Imm(horrible_cast<uintptr_t>(&LuaDetour::FreeLuaDetourCallInstance)));

			LoadASMRegisters(a);
			a.jmp(x86::dword_ptr_abs(reinterpret_cast<uintptr_t>(&mOriginalFunction)));
			
			mRuntime._add(&mCleanupAndJmpFunction, &code);
		}

        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&mOriginalFunction, mDetourFunction);
        DetourTransactionCommit();
	}

	~LuaDetour()
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&mOriginalFunction, mDetourFunction);
        DetourTransactionCommit();

		mRuntime._release(mDetourFunction);
		mDetourFunction = nullptr;

		mRuntime._release(mCleanupAndRetFunction);
		mCleanupAndRetFunction = nullptr;

		mRuntime._release(mCleanupAndJmpFunction);
		mCleanupAndJmpFunction = nullptr;
	}
	
	LuaDetour(const LuaDetour&) = delete;
	LuaDetour& operator=(const LuaDetour&) = delete;
	LuaDetour(LuaDetour&&) = delete;
	LuaDetour& operator=(LuaDetour&&) = delete;

	uintptr_t NewLuaDetourCallInstance(uintptr_t return_address)
	{
		LuaDetourCallInstance* lua_detour_call_instance = nullptr;
		GetLuaSpore().ExecuteOnFreeState([this, &lua_detour_call_instance, return_address](sol::state_view s)
		{
			if (auto* fn = mLuaFunction.get(s))
			{
				lua_detour_call_instance = new LuaDetourCallInstance(*fn, return_address, reinterpret_cast<uintptr_t>(mOriginalFunction), reinterpret_cast<uintptr_t>(mCleanupAndRetFunction), reinterpret_cast<uintptr_t>(mCleanupAndJmpFunction));
			}
		});
		return lua_detour_call_instance ? lua_detour_call_instance->GetJitFunctionAddress() : 0;
	}

	void FreeLuaDetourCallInstance(const LuaDetourCallInstance* lua_detour_call_instance) const
	{
		assert(lua_detour_call_instance != nullptr);
		delete lua_detour_call_instance;
	}
private:
	LuaDetourFunctionInfo mFunctionInfo{};

	LuaMultiReference<sol::function> mLuaFunction{};

	JitRuntime mRuntime;

	void* mOriginalFunction = nullptr;
	void* mDetourFunction = nullptr;
	void* mCleanupAndRetFunction = nullptr;
	void* mCleanupAndJmpFunction = nullptr;
};

static void AddLuaDetour(const LuaDetourFunctionInfo function_info, const sol::function& fn)
{
	new LuaDetour(function_info, fn);
}

OnLuaInit(sol::state_view s, bool is_main_state)
{
	if (!is_main_state) return;

	s.new_usertype<LuaDetourFunctionInfo>(
		"DetourFunctionInfo",
		sol::call_constructor, sol::initializers(
			[](LuaDetourFunctionInfo* memory, const uintptr_t disk_address, const uintptr_t address = 0x0)
			{
				auto* self = new(memory) LuaDetourFunctionInfo();
				self->disk_address = disk_address;
				self->address = address != 0x0 ? address : disk_address;
			}
		),
		"disk_address", &LuaDetourFunctionInfo::disk_address,
		"address", &LuaDetourFunctionInfo::address,
		sol::meta_function::to_string, [](const LuaDetourFunctionInfo& info) { return eastl::string().sprintf("LuaDetourFunctionInfo address(0x%08X) disk_address(0x%08X)", info.address, info.disk_address); }
	);

	s["AddDetour"] = AddLuaDetour;
}

#endif