#pragma once

#include "common/Meta.h"

template<class T, FixedString REDName, bool CheckObjectType = !std::is_arithmetic_v<T>>
struct LuaRED
{
    static constexpr char const* Name = REDName;
    
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
    {
        return make_object(aLua.Get(), *static_cast<T*>(aResult.value));
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        result.type = m_pRtti;
        if (aObject == sol::nil)
        {
            result.value = apAllocator->New<T>();
        }
        else if (!CheckObjectType || aObject.is<T>())
        {
            result.value = apAllocator->New<T>(aObject.as<T>());
        }

        return result;
    }

    void ToRED(sol::object aObject, RED4ext::CStackType* apType)
    {
        if (aObject == sol::nil)
        {
            *reinterpret_cast<T*>(apType->value) = T{};
        }
        else if (!CheckObjectType || aObject.is<T>())
        {
            *reinterpret_cast<T*>(apType->value) = aObject.as<T>();
        }
    }

    size_t Size() const noexcept
    {
        return sizeof(T);
    }

    bool Is(RED4ext::IRTTIType* apRtti) const
    {
        if (!Resolve())
            return false;
        
        return apRtti == m_pRtti;
    }

protected:

    bool Resolve() const
    {
        if (m_pRtti)
            return true;

        auto* pRtti = RED4ext::CRTTISystem::Get();
        m_pRtti = pRtti->GetType(RED4ext::FNV1a(Name));

        return m_pRtti != nullptr;
    }

    mutable RED4ext::IRTTIType* m_pRtti{nullptr};
};
