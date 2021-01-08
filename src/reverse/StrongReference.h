#pragma once

#include "Type.h"
#include "BasicTypes.h"

struct StrongReference : Type
{
    StrongReference(sol::state_view aView, RED4ext::Handle<RED4ext::IScriptable> aStrongHandle);
    ~StrongReference();
    
protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    
    RED4ext::Handle<RED4ext::IScriptable> m_strongHandle;
};
