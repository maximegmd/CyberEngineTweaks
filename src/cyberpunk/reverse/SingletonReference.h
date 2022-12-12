#pragma once

#include "Type.h"

struct SingletonReference : ClassType
{
    SingletonReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::CBaseRTTIType* apClass);
    ~SingletonReference() override;

protected:
    RED4ext::ScriptInstance GetHandle() const override;
};
