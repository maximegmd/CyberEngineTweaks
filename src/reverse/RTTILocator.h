#pragma once

struct RTTILocator
{
    RTTILocator(RED4ext::CName aName);

    operator RED4ext::IRTTIType*();

private:

    const RED4ext::CName m_name;
    RED4ext::IRTTIType* m_pRtti;
};
