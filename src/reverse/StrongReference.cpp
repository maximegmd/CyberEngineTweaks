#include <stdafx.h>

#include "StrongReference.h"

StrongReference::StrongReference(sol::state_view aView, RED4ext::Handle<RED4ext::IScriptable> aStrongHandle)
    : ClassType(aView, nullptr)
    , m_strongHandle(aStrongHandle)
{
    if (aStrongHandle)
    {
        m_pType = aStrongHandle->GetType();
    }
}

RED4ext::ScriptInstance StrongReference::GetHandle()
{
    return m_strongHandle.instance;
}
