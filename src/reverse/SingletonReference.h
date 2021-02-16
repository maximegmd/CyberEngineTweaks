#pragma once

#include "Type.h"

struct SingletonReference : ClassType
{
    SingletonReference(const Lockable<sol::state_view, std::recursive_mutex>& aView, RED4ext::IRTTIType* apClass);
    ~SingletonReference();
    
protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    uint64_t m_hash;
};
