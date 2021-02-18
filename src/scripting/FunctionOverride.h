#pragma once

struct Scripting;

struct FunctionOverride
{
    struct Context
    {
        sol::protected_function ScriptFunction;
        sol::environment Environment;
        bool Forward;
    };

    FunctionOverride(Scripting* apScripting, Options& aOptions);
    ~FunctionOverride();

    void* MakeExecutable(uint8_t* apData, size_t aSize);
    void Clear();

    void Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
                  bool aAbsolute, sol::protected_function aFunction, sol::this_environment aThisEnv);

protected:

    static void HandleOverridenFunction(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut, int64_t a4, RED4ext::CClassFunction* apFunction);
    static bool HookRunPureScriptFunction(RED4ext::CClassFunction* apFunction, RED4ext::CScriptStack* apContext, void* a3);

private:

    void Hook(Options& aOptions) const;

    enum
    {
        kExecutableSize = 1 << 20
    };

    struct CallChain
    {
        RED4ext::CClassFunction* Trampoline;
        Scripting* pScripting;
        TiltedPhoques::Vector<TiltedPhoques::UniquePtr<Context>> Calls;
    };

    void* m_pBufferStart;
    void* m_pBuffer;
    size_t m_size{ kExecutableSize };
    TiltedPhoques::Map<RED4ext::CClassFunction*, CallChain> m_functions;
    Scripting* m_pScripting;
    std::shared_mutex m_lock;
};