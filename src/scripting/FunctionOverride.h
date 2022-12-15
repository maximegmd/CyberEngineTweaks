#pragma once

#if GAME_CYBERPUNK

struct Scripting;

struct FunctionOverride
{
    struct Context
    {
        sol::protected_function ScriptFunction;
        sol::environment Environment;
        bool Forward;
    };

    struct CallChain
    {
        RED4ext::CBaseFunction* Trampoline;
        Scripting* pScripting;
        TiltedPhoques::Vector<TiltedPhoques::UniquePtr<Context>> Before;
        TiltedPhoques::Vector<TiltedPhoques::UniquePtr<Context>> After;
        TiltedPhoques::Vector<TiltedPhoques::UniquePtr<Context>> Overrides;
        bool IsEmpty;
        bool CollectGarbage;
    };

    FunctionOverride(Scripting* apScripting);
    ~FunctionOverride();

    void* MakeExecutable(const uint8_t* apData, size_t aSize);
    void Refresh();
    void Clear();

    void Override(
        const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction, sol::environment aEnvironment, bool aAbsolute, bool aAfter = false,
        bool aCollectGarbage = false);

protected:
    static void CopyFunctionDescription(RED4ext::CBaseFunction* aFunc, RED4ext::CBaseFunction* aRealFunc, bool aForceNative);
    static void HandleOverridenFunction(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, void* apOut, void* a4, RED4ext::CClassFunction* apFunction);
    static bool HookRunPureScriptFunction(RED4ext::CClassFunction* apFunction, RED4ext::CScriptStack* apStack, RED4ext::CStackFrame* a3);
    static void* HookCreateFunction(void* apMemoryPool, size_t aSize);
    static bool ExecuteChain(
        const CallChain& aChain, std::shared_lock<std::shared_mutex>& aLock, RED4ext::IScriptable* apContext, TiltedPhoques::Vector<sol::object>* apOrigArgs,
        RED4ext::CStackType* apResult, TiltedPhoques::Vector<RED4ext::CStackType>* apOutArgs, RED4ext::CScriptStack* apStack, RED4ext::CStackFrame* apFrame, char* apCode,
        uint8_t aParam);
    static sol::function WrapNextOverride(
        const CallChain& aChain, int aStep, sol::state& aLuaState, sol::object& aLuaContext, TiltedPhoques::Vector<sol::object>& aLuaArgs, RED4ext::CBaseFunction* apRealFunction,
        RED4ext::IScriptable* apRealContext, std::shared_lock<std::shared_mutex>& aLock);

private:
    enum
    {
        kExecutableSize = 1 << 20
    };

    void Hook() const;

    void* m_pBufferStart;
    void* m_pBuffer;
    size_t m_size{kExecutableSize};
    TiltedPhoques::Map<RED4ext::CBaseFunction*, CallChain> m_functions;
    TiltedPhoques::Map<RED4ext::CBaseFunction*, RED4ext::CBaseFunction*> m_trampolines;
    Scripting* m_pScripting;
    std::shared_mutex m_lock;
};

#endif