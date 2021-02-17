#include <stdafx.h>

#include "WeakReference.h"

#include "CET.h"

WeakReference::WeakReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                             RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle)
    : ClassType(aView, nullptr)
    , m_weakHandle(std::move(aWeakHandle))
{
    auto ref = m_weakHandle.Lock();
    if (ref)
    {
        m_pType = ref->GetType();
    }
}

WeakReference::~WeakReference()
{
    // Nasty hack so that the Lua VM doesn't try to release game memory on shutdown
    if (!CET::IsRunning())
    {
        m_weakHandle.instance = nullptr;
        m_weakHandle.refCount = nullptr;
    }
}


RED4ext::ScriptInstance WeakReference::GetHandle()
{
    const auto ref = m_weakHandle.Lock();
    if (ref)
    {
        return m_weakHandle.instance;
    }

    return nullptr;
}
