#pragma once

#include "Type.h"

struct SingletonReference : ClassType
{
    SingletonReference(sol::state_view aView, RED4ext::IRTTIType* apClass);
    ~SingletonReference();
    
protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    uint64_t m_hash;
};
