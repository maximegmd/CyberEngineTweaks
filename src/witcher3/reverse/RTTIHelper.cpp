#include <stdafx.h>

#include "RTTIHelper.h"

namespace
{
std::unique_ptr<RTTIHelper> s_pInstance{nullptr};
}

sol::function RTTIHelper::ResolveFunction(const std::string& acFuncName)
{
    return sol::nil;
}

void RTTIHelper::Initialize(const LockableState& acLua, LuaSandbox& apSandbox)
{
    if (!s_pInstance)
    {
        s_pInstance.reset(new RTTIHelper(acLua, apSandbox));
    }
}

void RTTIHelper::PostInitialize()
{
    if (s_pInstance)
    {
        s_pInstance->InitializeRuntime();
    }
}

void RTTIHelper::Shutdown()
{
    // Since m_resolvedFunctions contains references to Lua state
    // it's important to call destructor while Lua state exists.
    // Otherwise sol will throw invalid reference exception.
    s_pInstance.reset(nullptr);
}

RTTIHelper& RTTIHelper::Get()
{
    return *s_pInstance;
}

RTTIHelper::RTTIHelper(const LockableState& acpLua, LuaSandbox& apSandbox)
    : m_lua(acpLua)
    , m_sandbox(apSandbox)
{
}

void RTTIHelper::InitializeRuntime()
{
}
