#pragma once

#include "Type.h"
#include "BasicTypes.h"

struct WeakReference : ClassType
{
    WeakReference(sol::state_view aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle);

protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    
    RED4ext::WeakHandle<RED4ext::IScriptable> m_weakHandle;
};
