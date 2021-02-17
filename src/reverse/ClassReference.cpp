#include <stdafx.h>

#include "ClassReference.h"

ClassReference::ClassReference(const TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aView,
                               RED4ext::IRTTIType* apClass, RED4ext::ScriptInstance apInstance)
    : ClassType(aView, apClass)
{
    // Hack for now until we use their allocators, classes can actually be pointers to structs
    // GI just happens to be a 8-byte struct with only a pointer in it
    m_pInstance = std::make_unique<uint8_t[]>(apClass->GetSize());
    memcpy(m_pInstance.get(), apInstance, apClass->GetSize());
}

RED4ext::ScriptInstance ClassReference::GetHandle()
{
    return m_pInstance.get();
}
