#include <stdafx.h>

#include "WeakReference.h"

WeakReference::WeakReference(sol::state_view aView, WeakHandle aWeakHandle)
    : Type(aView, static_cast<RED4ext::CClass*>(aWeakHandle.handle->GetParentType()))
    , m_weakHandle(aWeakHandle)
{
}

WeakReference::~WeakReference()
{
    // Someday maybe actually free memory
}

RED4ext::IScriptable* WeakReference::GetHandle()
{
    return m_weakHandle.handle;
}
