#pragma once

#include "Type.h"

struct ClassReference : ClassType
{
    ClassReference(const Lockable<sol::state_view, std::recursive_mutex>& aView, RED4ext::IRTTIType* apClass,
                   RED4ext::ScriptInstance apInstance);

    virtual RED4ext::ScriptInstance GetHandle();

private:
    std::unique_ptr<uint8_t[]> m_pInstance;
};
