#include <stdafx.h>

#include "StrongReference.h"

#include "CET.h"

StrongReference::StrongReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                                 RED4ext::Handle<RED4ext::IScriptable> aStrongHandle)
    : ClassType(aView, nullptr)
    , m_strongHandle(std::move(aStrongHandle))
{
    if (m_strongHandle)
    {
        m_pType = m_strongHandle->GetType();
    }
}

StrongReference::~StrongReference()
{
    // Nasty hack so that the Lua VM doesn't try to release game memory on shutdown
    if (!CET::IsRunning())
    {
        m_strongHandle.instance = nullptr;
        m_strongHandle.refCount = nullptr;
    }
}


RED4ext::ScriptInstance StrongReference::GetHandle()
{
    return m_strongHandle.instance;
}
