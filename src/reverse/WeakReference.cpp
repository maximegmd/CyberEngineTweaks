#include <stdafx.h>

#include "WeakReference.h"
#include "RTTILocator.h"

#include "CET.h"

WeakReference::WeakReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle)
    : ClassType(aView, nullptr)
    , m_weakHandle(std::move(aWeakHandle))
{
    const auto ref = m_weakHandle.Lock();
    if (ref)
    {
        m_pType = ref->GetType();
    }
}

WeakReference::WeakReference(
    const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle,
    RED4ext::CRTTIWeakHandleType* apWeakHandleType)
    : ClassType(aView, nullptr)
    , m_weakHandle(std::move(aWeakHandle))
{
    const auto cRef = m_weakHandle.Lock();
    if (!cRef)
        return;

    const auto cpClass = reinterpret_cast<RED4ext::CClass*>(apWeakHandleType->GetInnerType());

    thread_local static RTTILocator sIScriptableType{RED4ext::FNV1a64("IScriptable")};
    m_pType = cpClass->IsA(sIScriptableType) ? cRef->GetType() : cRef->GetNativeType();
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

RED4ext::ScriptInstance WeakReference::GetHandle() const
{
    const auto ref = m_weakHandle.Lock();
    if (ref)
    {
        return m_weakHandle.instance;
    }

    return nullptr;
}

RED4ext::ScriptInstance WeakReference::GetValuePtr() const
{
    return const_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(&m_weakHandle);
}
