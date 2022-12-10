#pragma once

#include "Type.h"

struct ClassReference : ClassType
{
    ClassReference(const TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aView, RED4ext::CBaseRTTIType* apClass, RED4ext::ScriptInstance apInstance);
    ClassReference(ClassReference&& aOther) noexcept;
    ~ClassReference() override;

    RED4ext::ScriptInstance GetHandle() const override;
    RED4ext::ScriptInstance GetValuePtr() const override;

private:
    RED4ext::ScriptInstance m_pInstance;
};
