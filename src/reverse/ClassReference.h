#pragma once

#include "Type.h"

struct ClassReference : ClassType
{
    ClassReference(const TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aView, RED4ext::rtti::IType* apClass, void* apInstance);
    ClassReference(ClassReference&& aOther) noexcept;
    ~ClassReference() override;

    void* GetHandle() const override;
    void* GetValuePtr() const override;

private:
    void* m_pInstance;
};
