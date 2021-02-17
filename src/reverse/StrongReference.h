#pragma once

#include "Type.h"

struct StrongReference : ClassType
{
    StrongReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                    RED4ext::Handle<RED4ext::IScriptable> aStrongHandle);
    virtual ~StrongReference();

protected:

    virtual RED4ext::ScriptInstance GetHandle();
    
private:
    friend struct Scripting;
    friend struct TweakDB;
    
    RED4ext::Handle<RED4ext::IScriptable> m_strongHandle;
};
