#pragma once

#include "Enum.h"
#include "ClassReference.h"
#include "LuaRED.h"
#include "RTTIHelper.h"

namespace TiltedPhoques
{
struct Allocator;
}

namespace Converter
{
size_t Size(RED4ext::CBaseRTTIType* apRtti);
sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua);
RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);
} // namespace Converter

// Specialization manages special case implicit casting
struct CNameConverter : LuaRED<CName, "CName">
{
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        if (aObject != sol::nil && aObject.get_type() == sol::type::string) // CName from String implicit cast
        {
            sol::state_view v(aObject.lua_state());
            std::string str = v["tostring"](aObject);
            result.type = apRtti;
            result.value = apAllocator->New<CName>(str);
        }
        else
        {
            return LuaRED<CName, "CName">::ToRED(aObject, apRtti, apAllocator);
        }

        return result;
    }
};

// Specialization manages special case implicit casting for TweakDBID
struct TweakDBIDConverter : LuaRED<TweakDBID, "TweakDBID">
{
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        if (aObject != sol::nil && aObject.get_type() == sol::type::string)
        {
            result.type = apRtti;
            result.value = apAllocator->New<TweakDBID>(aObject.as<std::string>());
        }
        else
        {
            return LuaRED<TweakDBID, "TweakDBID">::ToRED(aObject, apRtti, apAllocator);
        }

        return result;
    }
};

// Specialization manages wrapping and converting RT_Enum
struct EnumConverter : LuaRED<Enum, "Enum">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua) { return make_object(aLua.Get(), Enum(aResult)); }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        if (aObject.is<Enum>())
        {
            auto* pEnum = aObject.as<Enum*>();
            if (pEnum->GetType() == apRtti)
            {
                pEnum->Set(result, apAllocator);
            }
            else // The enum type we were passed isn't the same
            {
                result.type = apRtti;
                result.value = nullptr;
            }
        }
        else if (aObject == sol::nil)
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apRtti);
            Enum en(enumType, 0);
            en.Set(result, apAllocator);
        }
        else if (aObject.get_type() == sol::type::number) // Enum from number cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apRtti);
            Enum en(enumType, aObject.as<uint32_t>());
            en.Set(result, apAllocator);
        }
        else if (aObject.get_type() == sol::type::string) // Enum from string cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apRtti);
            sol::state_view v(aObject.lua_state());
            std::string str = v["tostring"](aObject);
            Enum en(enumType, str);
            en.Set(result, apAllocator);
        }
        else
        {
            // Probably not going to like this but ok
            result.type = apRtti;
            result.value = nullptr;
        }

        return result;
    }

    size_t Size() const noexcept { return m_pRtti ? m_pRtti->GetSize() : 0; }

    bool Is(RED4ext::CBaseRTTIType* apRtti) const
    {
        if (apRtti->GetType() == RED4ext::ERTTIType::Enum)
        {
            m_pRtti = apRtti;
            return true;
        }

        return false;
    }
};

// Specialization manages wrapping and converting RT_BitField
struct BitFieldConverter : LuaRED<uint64_t, "BitField">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
    {
        uint64_t value;

        switch (m_pRtti->GetSize())
        {
        case sizeof(uint8_t): value = *static_cast<uint8_t*>(aResult.value); break;
        case sizeof(uint16_t): value = *static_cast<uint16_t*>(aResult.value); break;
        case sizeof(uint32_t): value = *static_cast<uint32_t*>(aResult.value); break;
        case sizeof(uint64_t): value = *static_cast<uint64_t*>(aResult.value); break;
        }

        auto res{aLua.Get().script(fmt::format("return {}ull", value))};
        assert(res.valid());
        return res.get<sol::object>();
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        result.type = apRtti;
        result.value = apAllocator->Allocate(apRtti->GetSize());

        uint64_t value = 0;

        if (aObject.get_type() == sol::type::number)
        {
            value = aObject.as<uint32_t>();
        }
        else if (IsLuaCData(aObject))
        {
            sol::state_view v(aObject.lua_state());
            std::string str = v["tostring"](aObject);
            value = std::stoull(str);
        }

        switch (m_pRtti->GetSize())
        {
        case sizeof(uint8_t): *static_cast<uint8_t*>(result.value) = static_cast<uint8_t>(value); break;
        case sizeof(uint16_t): *static_cast<uint16_t*>(result.value) = static_cast<uint16_t>(value); break;
        case sizeof(uint32_t): *static_cast<uint32_t*>(result.value) = static_cast<uint32_t>(value); break;
        case sizeof(uint64_t): *static_cast<uint64_t*>(result.value) = value; break;
        }

        return result;
    }

    size_t Size() const noexcept { return m_pRtti ? m_pRtti->GetSize() : 0; }

    bool Is(RED4ext::CBaseRTTIType* apRtti) const
    {
        if (apRtti->GetType() == RED4ext::ERTTIType::BitField)
        {
            m_pRtti = apRtti;
            return true;
        }

        return false;
    }
};

// Specialization manages wrapping RT_Class
struct ClassConverter : LuaRED<ClassReference, "ClassReference">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
    {
        return make_object(aLua.Get(), ClassReference(aLua, aResult.type, aResult.value));
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        result.type = apRtti;
        if (aObject.is<ClassReference>())
        {
            result.value = aObject.as<ClassReference*>()->GetHandle();
        }
        else if (aObject == sol::nil)
        {
            result.value = RTTIHelper::Get().NewInstance(apRtti, sol::nullopt, apAllocator);
        }
        // Disabled until new allocator is implemented
        // Current implementation can leak
        // else if (aObject.get_type() == sol::type::table)
        //{
        //    // The implicit table to instance conversion `Game.FindEntityByID({ hash = 1 })` has potential issue:
        //    // When the overloaded function takes an array and an object for the same arg the implicit conversion
        //    // can produce an empty instance making the unwanted overload callable. So for better experience it's
        //    // important to distinguish between linear array and array of props.
        //
        //    // Size check excludes non-empty linear arrays since only the table with sequential and integral keys
        //    // has size (length). And iterator check excludes empty tables `{}`.
        //    sol::table props = aObject.as<sol::table>();
        //    if (props.size() == 0 && props.begin() != props.end())
        //        result.value = RTTIHelper::Get().NewInstance(apRtti, props, apAllocator);
        //    else
        //        result.value = nullptr;
        //}
        else
        {
            result.value = nullptr;
        }

        return result;
    }

    size_t Size() const noexcept { return m_pRtti ? m_pRtti->GetSize() : 0; }

    bool Is(RED4ext::CBaseRTTIType* apRtti) const
    {
        if (apRtti->GetType() == RED4ext::ERTTIType::Class)
        {
            m_pRtti = apRtti;
            return true;
        }

        return false;
    }
};

// Specialization manages wrapping RT_***
struct RawConverter : LuaRED<UnknownType, "UnknownType">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
    {
        return make_object(aLua.Get(), UnknownType(aLua, aResult.type, aResult.value));
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator*)
    {
        RED4ext::CStackType result;
        result.type = apRtti;
        if (aObject.is<UnknownType>())
        {
            result.value = aObject.as<UnknownType*>()->GetHandle();
        }
        else
        {
            result.value = nullptr;
        }

        return result;
    }

    size_t Size() const noexcept { return m_pRtti ? m_pRtti->GetSize() : 0; }

    bool Is(RED4ext::CBaseRTTIType* apRtti) const
    {
        m_pRtti = apRtti;
        return true;
    }
};
