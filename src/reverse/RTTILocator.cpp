#include "stdafx.h"

#include "RTTILocator.h"

RTTILocator::RTTILocator(RED4ext::CName aName)
    : m_name(aName)
{
}

RTTILocator::operator RED4ext::CBaseRTTIType*()
{
    if (m_pRtti)
        return m_pRtti;

    auto* pRtti = RED4ext::CRTTISystem::Get();
    if (pRtti == nullptr)
    {
        return nullptr;
    }

    m_pRtti = pRtti->GetType(m_name);
    return m_pRtti;
}
