#include <stdafx.h>

#include "WeakReference.h"

WeakReference::WeakReference(sol::state_view aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle)
    : Type(aView, static_cast<RED4ext::CClass*>(aWeakHandle->GetParentType()))
    , m_weakHandle(aWeakHandle)
{
}

WeakReference::~WeakReference()
{
    // Someday maybe actually free memory
}

RED4ext::ScriptInstance WeakReference::GetHandle()
{
    return m_weakHandle.instance;
}
