#pragma once

#include "Type.h"

struct WeakReference : ClassType
{
    WeakReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                  RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle);
    WeakReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                  RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle,
                  RED4ext::CWeakHandle* apWeakHandleType);
    virtual ~WeakReference();

protected:

    virtual RED4ext::ScriptInstance GetHandle() const override;
    
private:
    friend struct Scripting;
    friend struct TweakDB;
    
    RED4ext::WeakHandle<RED4ext::IScriptable> m_weakHandle;
};
