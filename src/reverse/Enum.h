#pragma once

#include "LuaRED.h"

struct Enum
{
    Enum(const RED4ext::CEnum*, const std::string& value);
    Enum(const RED4ext::CEnum*, uint32_t value);
    Enum(const RED4ext::CStackType& stackType);
    Enum(const std::string& typeName, const std::string& value);
    Enum(const std::string& typeName, uint32_t value);

    void Get(const RED4ext::CStackType& stackType);
    void Set(RED4ext::CStackType& stackType, TiltedPhoques::Allocator* apAllocator);

    // Returns the enum value by name
    std::string GetValueName() const;

    // Sets value by name in the enum list
    void SetValueByName(const std::string& value);

    // Sets by value verified against enum list
    void SetValueSafe(uint64_t value);

    std::string ToString() const;

    const RED4ext::CEnum* GetType() const;

protected:
    const RED4ext::CEnum*	m_type{ nullptr };
    uint64_t				m_value{ 0 };
};

template<>
struct LuaRED<Enum, "Enum">
{
    sol::object ToLua(RED4ext::CStackType& aResult, sol::state_view aLua)
    {
        return make_object(aLua, Enum(aResult));
    }

    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
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
        else if (aObject.get_type() == sol::type::number) // Enum from number cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apRtti);
            if (aObject != sol::nil)
            {
                Enum en(enumType, aObject.as<uint32_t>());
                en.Set(result, apAllocator);
            }
        }
        else if (aObject.get_type() == sol::type::string) // Enum from string cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apRtti);
            if (aObject != sol::nil)
            {
                sol::state_view v(aObject.lua_state());
                std::string str = v["tostring"](aObject);
                Enum en(enumType, str);
                en.Set(result, apAllocator);
            }
        }
        else
        {
            // Probably not going to like this but ok
            result.type = apRtti;
            result.value = nullptr;
        }

        return result;
    }

    size_t Size() const noexcept
    {
        return m_pRtti ? m_pRtti->GetSize() : 0;
    }

    bool Is(RED4ext::IRTTIType* apRtti) const
    {
        if (apRtti->GetType() == RED4ext::ERTTIType::Enum)
        {
            m_pRtti = apRtti;
            return true;
        }

        return false;
    }

private:
    mutable RED4ext::IRTTIType* m_pRtti{ nullptr };
};
