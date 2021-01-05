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
    sol::object ToLua(RED4ext::CStackType& aResult, sol::state_view aLua);
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);
}

// Specialization manages special case implicit casting
struct CNameConverter : public LuaRED<CName, "CName">
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
};

// Specialization manages wrapping and converting RT_Enum
struct EnumConverter : public LuaRED<Enum, "Enum">
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
};

// Specialization manages wrapping RT_Class
struct ClassConverter : public LuaRED<ClassReference, "ClassReference">
{
	sol::object ToLua(RED4ext::CStackType& aResult, sol::state_view aLua)
	{
		return make_object(aLua, ClassReference(aLua, static_cast<RED4ext::CClass*>(aResult.type), *static_cast<void**>(aResult.value)));
	}

	RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
	{
		RED4ext::CStackType result;
		result.type = apRtti;
		if (aObject.is<ClassReference>())
		{
			result.value = apAllocator->New<void*>(aObject.as<ClassReference*>()->GetHandle());
		}
		else
		{
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
		if (apRtti->GetType() == RED4ext::ERTTIType::Class)
		{
			m_pRtti = apRtti;
			return true;
		}

		return false;
	}
};
