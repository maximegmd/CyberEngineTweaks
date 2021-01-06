#pragma once

#include "Type.h"

struct SingletonReference : Type
{
    SingletonReference(sol::state_view aView, RED4ext::CClass* apClass);
    ~SingletonReference();
    
protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    uint64_t m_hash;
};
