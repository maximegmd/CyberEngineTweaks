#pragma once

#include "Type.h"
#include "BasicTypes.h"

struct WeakReference : Type
{
    WeakReference(sol::state_view aView, WeakHandle aWeakHandle);
    ~WeakReference();
    
protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    
    WeakHandle m_weakHandle;
};
