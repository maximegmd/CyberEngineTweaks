#pragma once

#include "common/Meta.h"
#include "Utils.h"

template <class T, FixedString REDName> struct LuaRED
{
    static constexpr char const* Name = REDName;

    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
    {
        if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(uint64_t))
        {
            constexpr auto format{std::is_signed_v<T> ? "return {}ll" : "return {}ull"};
            auto res{aLua.Get().script(fmt::format(format, *static_cast<T*>(aResult.value)))};
            assert(res.valid());
            return res.get<sol::object>();
        }
        else
        {
            return make_object(aLua.Get(), *static_cast<T*>(aResult.value));
        }
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType*, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        result.type = m_pRtti;
        if (aObject == sol::nil)
        {
            result.value = apAllocator->New<T>();
        }
        else if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(uint64_t))
        {
            if (aObject.get_type() == sol::type::number)
            {
                sol::state_view v(aObject.lua_state());
                double value = v["tonumber"](aObject);
                result.value = apAllocator->New<T>(static_cast<T>(value));
            }
            else if (IsLuaCData(aObject))
            {
                sol::state_view v(aObject.lua_state());
                std::string str = v["tostring"](aObject);
                if constexpr (std::is_signed_v<T>)
                    result.value = apAllocator->New<T>(std::stoll(str));
                else
                    result.value = apAllocator->New<T>(std::stoull(str));
            }
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            if (aObject.get_type() == sol::type::boolean)
                result.value = apAllocator->New<T>(aObject.as<T>());
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            if (aObject.get_type() == sol::type::number)
            {
                result.value = apAllocator->New<T>(aObject.as<T>());
            }
            else if (IsLuaCData(aObject))
            {
                sol::state_view v(aObject.lua_state());
                double value = v["tonumber"](aObject);
                result.value = apAllocator->New<T>(static_cast<T>(value));
            }
        }
        else if (aObject.is<T>())
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
        else if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(uint64_t))
        {
            if (aObject.get_type() == sol::type::number)
            {
                sol::state_view v(aObject.lua_state());
                double value = v["tonumber"](aObject);
                *reinterpret_cast<T*>(apType->value) = static_cast<T>(value);
            }
            else if (IsLuaCData(aObject))
            {
                sol::state_view v(aObject.lua_state());
                std::string str = v["tostring"](aObject);
                if constexpr (std::is_signed_v<T>)
                    *reinterpret_cast<T*>(apType->value) = std::stoll(str);
                else
                    *reinterpret_cast<T*>(apType->value) = std::stoull(str);
            }
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            if (aObject.get_type() == sol::type::boolean)
                *reinterpret_cast<T*>(apType->value) = aObject.as<T>();
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            if (aObject.get_type() == sol::type::number)
            {
                *reinterpret_cast<T*>(apType->value) = aObject.as<T>();
            }
            else if (IsLuaCData(aObject))
            {
                sol::state_view v(aObject.lua_state());
                double value = v["tonumber"](aObject);
                *reinterpret_cast<T*>(apType->value) = static_cast<T>(value);
            }
        }
        else if (aObject.is<T>())
        {
            *reinterpret_cast<T*>(apType->value) = aObject.as<T>();
        }
    }

    size_t Size() const noexcept { return sizeof(T); }

    bool Is(RED4ext::CBaseRTTIType* apRtti) const
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
        m_pRtti = pRtti->GetType(RED4ext::FNV1a64(Name));

        return m_pRtti != nullptr;
    }

    mutable RED4ext::CBaseRTTIType* m_pRtti{nullptr};
};
