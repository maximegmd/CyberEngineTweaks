#pragma once

#include "Type.h"

struct WeakReference : ClassType
{
    WeakReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle);
    WeakReference(
        const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::WeakHandle<RED4ext::IScriptable> aWeakHandle,
        RED4ext::CRTTIWeakHandleType* apWeakHandleType);
    ~WeakReference() override;

protected:
    RED4ext::ScriptInstance GetHandle() const override;
    RED4ext::ScriptInstance GetValuePtr() const override;

private:
    friend struct Scripting;
    friend struct TweakDB;

    RED4ext::WeakHandle<RED4ext::IScriptable> m_weakHandle;
};
