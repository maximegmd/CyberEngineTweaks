#pragma once

#include "Type.h"

struct WeakReference : ClassType
{
    WeakReference(const Lockable<sol::state_view, std::recursive_mutex>& aView,
                  RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle);
    virtual ~WeakReference();

protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    friend struct TweakDB;
    
    RED4ext::WeakHandle<RED4ext::IScriptable> m_weakHandle;
};
