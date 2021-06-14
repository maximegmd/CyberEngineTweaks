#pragma once

#include "Type.h"

struct SingletonReference : ClassType
{
    SingletonReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                       RED4ext::IRTTIType* apClass);
    ~SingletonReference();

protected:
    RED4ext::ScriptInstance GetHandle() override;
};
