#pragma once

struct RTTILocator
{
    RTTILocator(RED4ext::CName aName);

    operator RED4ext::CBaseRTTIType*();

private:
    const RED4ext::CName m_name;
    RED4ext::CBaseRTTIType* m_pRtti = nullptr;
};
