#pragma once

struct RTTILocator
{
    RTTILocator(RED4ext::CName aName);

    operator RED4ext::rtti::IType*();

private:
    const RED4ext::CName m_name;
    RED4ext::rtti::IType* m_pRtti = nullptr;
};
