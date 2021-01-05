#include <stdafx.h>

#include "ClassReference.h"

ClassReference::ClassReference(sol::state_view aView, RED4ext::CClass* apClass, void* apInstance)
    : Type(std::move(aView), apClass)
    , m_pInstance(apInstance)
{
}

ClassReference::~ClassReference() = default;

RED4ext::IScriptable* ClassReference::GetHandle()
{
    return reinterpret_cast<RED4ext::IScriptable*>(m_pInstance);
}
