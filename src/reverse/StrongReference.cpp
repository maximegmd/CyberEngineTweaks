#include <stdafx.h>

#include "StrongReference.h"

StrongReference::StrongReference(sol::state_view aView, StrongHandle aStrongHandle)
    : Type(aView, aStrongHandle.handle->GetClass())
    , m_strongHandle(aStrongHandle)
{
}

StrongReference::~StrongReference()
{
    // Someday maybe actually free memory
}

RED4ext::REDreverse::Scripting::IScriptable* StrongReference::GetHandle()
{
    return m_strongHandle.handle;
}
