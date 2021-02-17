#pragma once

struct Scripting;

struct FunctionOverride
{
    struct Context
    {
        sol::protected_function ScriptFunction;
        RED4ext::CClassFunction* RealFunction{nullptr};
        RED4ext::CClassFunction* FunctionDefinition{nullptr};
        sol::environment Environment;
        Scripting* pScripting;
        bool Forward;
    };

    FunctionOverride(Scripting* apScripting, Options& aOptions);
    ~FunctionOverride();

    void* MakeExecutable(uint8_t* apData, size_t aSize);
    void Clear();

    void Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
                  bool aAbsolute, sol::protected_function aFunction, sol::this_environment aThisEnv);

protected:

    static void HandleOverridenFunction(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut,
                                        int64_t a4, struct Context* apCookie);

private:

    void Hook(Options& aOptions) const;

    enum
    {
        kExecutableSize = 1 << 20
    };

    struct InternalOverride
    {
        RED4ext::CClassFunction* OldFunction{nullptr};
        RED4ext::CClassFunction* NewFunction{nullptr};
        TiltedPhoques::UniquePtr<Context> Context;
    };

    void* m_pBufferStart;
    void* m_pBuffer;
    size_t m_size{ kExecutableSize };
    TiltedPhoques::Vector<InternalOverride> m_overrides;
    Scripting* m_pScripting;
};