#include <stdafx.h>

#include "ClassReference.h"

ClassReference::ClassReference(sol::state_view aView, RED4ext::CClass* apClass, void* instance)
    : Type(std::move(aView), apClass)
    , m_instance(instance)
{
}

ClassReference::~ClassReference() = default;

RED4ext::IScriptable* ClassReference::GetHandle()
{
    return reinterpret_cast<RED4ext::IScriptable*>(m_instance);
}
