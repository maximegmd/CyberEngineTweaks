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
    void* GetHandle() const override;
    void* GetValuePtr() const override;
    RED4ext::rtti::IType* GetValueType() const override;

private:
    friend struct Scripting;
    friend struct TweakDB;

    RED4ext::WeakHandle<RED4ext::IScriptable> m_weakHandle;
    RED4ext::CRTTIWeakHandleType* m_pHandleType;
};
