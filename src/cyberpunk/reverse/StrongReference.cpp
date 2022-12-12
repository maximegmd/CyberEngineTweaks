#include <stdafx.h>

#include "StrongReference.h"
#include "RTTILocator.h"

#include "EngineTweaks.h"

static RTTILocator s_sIScriptableType{RED4ext::FNV1a64("IScriptable")};

StrongReference::StrongReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::Handle<RED4ext::IScriptable> aStrongHandle)
    : ClassType(aView, nullptr)
    , m_strongHandle(std::move(aStrongHandle))
{
    if (m_strongHandle)
    {
        m_pType = m_strongHandle->GetType();
    }
}

StrongReference::StrongReference(
    const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::Handle<RED4ext::IScriptable> aStrongHandle, RED4ext::CRTTIHandleType* apStrongHandleType)
    : ClassType(aView, nullptr)
    , m_strongHandle(std::move(aStrongHandle))
{
    if (m_strongHandle)
    {
        auto const cpClass = reinterpret_cast<RED4ext::CClass*>(apStrongHandleType->GetInnerType());

        m_pType = cpClass->IsA(s_sIScriptableType) ? m_strongHandle->GetType() : cpClass;
    }
}

StrongReference::~StrongReference()
{
    // Nasty hack so that the Lua VM doesn't try to release game memory on shutdown
    if (!EngineTweaks::IsRunning())
    {
        m_strongHandle.instance = nullptr;
        m_strongHandle.refCount = nullptr;
    }
}

RED4ext::ScriptInstance StrongReference::GetHandle() const
{
    return m_strongHandle.instance;
}

RED4ext::ScriptInstance StrongReference::GetValuePtr() const
{
    return const_cast<RED4ext::Handle<RED4ext::IScriptable>*>(&m_strongHandle);
}
