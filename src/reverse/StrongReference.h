#pragma once

#include "Type.h"

struct StrongReference : ClassType
{
    StrongReference(const Lockable<sol::state_view, std::recursive_mutex>& aView,
                    RED4ext::Handle<RED4ext::IScriptable> aStrongHandle);
    virtual ~StrongReference();

protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    friend struct TweakDB;
    
    RED4ext::Handle<RED4ext::IScriptable> m_strongHandle;
};
