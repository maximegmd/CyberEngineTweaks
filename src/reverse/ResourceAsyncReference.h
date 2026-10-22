#pragma once

#include "Type.h"

struct ResourceAsyncReference : ClassType
{
    ResourceAsyncReference(
        const TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aView, RED4ext::rtti::IType* apType, RED4ext::ResourceAsyncReference<void>* apReference);

    static uint64_t Hash(const std::string& aPath);

    void* GetHandle() const override;
    void* GetValuePtr() const override;

    uint64_t GetHash() const;
    sol::object GetLuaHash() const;

private:
    RED4ext::ResourceAsyncReference<void> m_reference;
};
