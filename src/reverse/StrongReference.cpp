#include <stdafx.h>

#include "StrongReference.h"

StrongReference::StrongReference(sol::state_view aView, RED4ext::Handle<RED4ext::IScriptable> aStrongHandle)
    : ClassType(aView, aStrongHandle->GetParentType())
    , m_strongHandle(aStrongHandle)
{
}

StrongReference::~StrongReference()
{
    // Someday maybe actually free memory
}

RED4ext::ScriptInstance StrongReference::GetHandle()
{
    return m_strongHandle.instance;
}
