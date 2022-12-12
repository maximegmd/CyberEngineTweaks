#include <stdafx.h>

#include "WeakReference.h"
#include "RTTILocator.h"

#include "EngineTweaks.h"

static RTTILocator s_sIScriptableType{RED4ext::FNV1a64("IScriptable")};

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
    const auto ref = m_weakHandle.Lock();
    if (ref)
    {
        auto const cpClass = reinterpret_cast<RED4ext::CClass*>(apWeakHandleType->GetInnerType());

        m_pType = cpClass->IsA(s_sIScriptableType) ? ref->GetType() : cpClass;
    }
}

WeakReference::~WeakReference()
{
    // Nasty hack so that the Lua VM doesn't try to release game memory on shutdown
    if (!EngineTweaks::IsRunning())
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
