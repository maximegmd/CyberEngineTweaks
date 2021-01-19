#include <stdafx.h>

#include "WeakReference.h"

WeakReference::WeakReference(sol::state_view aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle)
    : ClassType(aView, nullptr)
    , m_weakHandle(aWeakHandle)
{
    auto ref = aWeakHandle.Lock();
    if (ref)
    {
        m_pType = ref->GetType();
    }
}

RED4ext::ScriptInstance WeakReference::GetHandle()
{
    auto ref = m_weakHandle.Lock();
    if (ref)
    {
        return m_weakHandle.instance;
    }

    return nullptr;
}
