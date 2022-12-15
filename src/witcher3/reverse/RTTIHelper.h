#pragma once

struct LuaSandbox;
struct RTTIHelper
{
    using LockableState = TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref;

    sol::function ResolveFunction(const std::string& acFuncName);

    static void Initialize(const LockableState& acpLua, LuaSandbox& apSandbox);
    static void PostInitialize();
    static void Shutdown();
    static RTTIHelper& Get();

private:
    RTTIHelper(const LockableState& acLua, LuaSandbox& apSandbox);

    void InitializeRuntime();

    LockableState m_lua;
    LuaSandbox& m_sandbox;
};
