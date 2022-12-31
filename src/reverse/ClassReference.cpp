#include "stdafx.h"

#include "ClassReference.h"
#include "CET.h"

ClassReference::ClassReference(const TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aView, RED4ext::CBaseRTTIType* apClass, RED4ext::ScriptInstance apInstance)
    : ClassType(aView, apClass)
{
    m_pInstance = apClass->GetAllocator()->AllocAligned(apClass->GetSize(), apClass->GetAlignment()).memory;
    apClass->Construct(m_pInstance);
    apClass->Assign(m_pInstance, apInstance);
}

ClassReference::ClassReference(ClassReference&& aOther) noexcept
    : ClassType(aOther)
    , m_pInstance(aOther.m_pInstance)
{
    aOther.m_pInstance = nullptr;
}

ClassReference::~ClassReference()
{
    if (m_pInstance && CET::IsRunning())
    {
        m_pType->Destruct(m_pInstance);
        m_pType->GetAllocator()->Free(m_pInstance);
    }
}

RED4ext::ScriptInstance ClassReference::GetHandle() const
{
    return m_pInstance;
}

RED4ext::ScriptInstance ClassReference::GetValuePtr() const
{
    return GetHandle();
}
