#pragma once

#include "BasicTypes.h"
#include "Enum.h"
#include "ClassReference.h"
#include "LuaRED.h"

namespace TiltedPhoques {
    struct Allocator;
}

namespace Converter
{
    size_t Size(RED4ext::IRTTIType* apRtti);
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua);
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);
    void ToRED(sol::object aObject, RED4ext::CStackType* apType);
}

// Specialization manages special case implicit casting
struct CNameConverter : LuaRED<CName, "CName">
{
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        if (aObject.get_type() == sol::type::string && aObject != sol::nil) // CName from String implicit cast
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

	void ToRED(sol::object aObject, RED4ext::CStackType* apType)
    {
        if (aObject.get_type() == sol::type::string && aObject != sol::nil) // CName from String implicit cast
        {
            sol::state_view v(aObject.lua_state());
            std::string str = v["tostring"](aObject);
            *(CName*)apType->value = CName(str);
        }
        else
        {
            return LuaRED<CName, "CName">::ToRED(aObject, apType);
        }
    }
};

// Specialization manages wrapping and converting RT_Enum
struct EnumConverter : LuaRED<Enum, "Enum">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
	{
		return make_object(aLua.Get(), Enum(aResult));
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

	void ToRED(sol::object aObject, RED4ext::CStackType* apType)
    {
        if (aObject.is<Enum>())
        {
            auto* pEnum = aObject.as<Enum*>();
            if (pEnum->GetType() == apType->type)
            {
                pEnum->Set(*apType);
            }
        }
        else if (aObject.get_type() == sol::type::number) // Enum from number cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apType->type);
            if (aObject != sol::nil)
            {
                Enum en(enumType, aObject.as<uint32_t>());
                en.Set(*apType);
            }
        }
        else if (aObject.get_type() == sol::type::string) // Enum from string cast
        {
            auto* enumType = static_cast<RED4ext::CEnum*>(apType->type);
            if (aObject != sol::nil)
            {
                sol::state_view v(aObject.lua_state());
                std::string str = v["tostring"](aObject);
                Enum en(enumType, str);
                en.Set(*apType);
            }
        }
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
};

// Specialization manages wrapping RT_Class
struct ClassConverter : LuaRED<ClassReference, "ClassReference">
{
    sol::object ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
	{
		return make_object(aLua.Get(), ClassReference(aLua, aResult.type, aResult.value));
	}

	RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
    {
        RED4ext::CStackType result;
        result.type = apRtti;
        if (aObject.is<ClassReference>())
        {
            result.value = aObject.as<ClassReference*>()->GetHandle();
        }
        else
        {
            result.value = nullptr;
        }

        return result;
    }

    void ToRED(sol::object aObject, RED4ext::CStackType* apType)
    {
        if (aObject.is<ClassReference>())
        {
            apType->value = aObject.as<ClassReference*>()->GetHandle();
        }
        else
        {
            apType->value = nullptr;
        }
    }

	size_t Size() const noexcept
	{
		return m_pRtti ? m_pRtti->GetSize() : 0;
	}

	bool Is(RED4ext::IRTTIType* apRtti) const
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

	RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
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

    void ToRED(sol::object aObject, RED4ext::CStackType* apType)
    {
        if (aObject.is<UnknownType>())
        {
            apType->value = aObject.as<UnknownType*>()->GetHandle();
        }
        else
        {
            apType->value = nullptr;
        }
    }

	size_t Size() const noexcept
	{
		return m_pRtti ? m_pRtti->GetSize() : 0;
	}

	bool Is(RED4ext::IRTTIType* apRtti) const
	{
		m_pRtti = apRtti;
		return true;
	}
};
