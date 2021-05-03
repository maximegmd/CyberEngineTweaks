#include <stdafx.h>

#include "SingletonReference.h"

SingletonReference::SingletonReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                                       RED4ext::IRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

SingletonReference::~SingletonReference() = default;
