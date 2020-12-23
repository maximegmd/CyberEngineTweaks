#pragma once

#include "common/Meta.h"
#include "RED4ext/REDreverse/RTTI/RTTISystem.hpp"
#include "RED4ext/REDhash.hpp"
#include "TiltedCore/Allocator.hpp"
#include "Converter.h"

template<class T, FixedString REDName>
struct LuaRED : Converter::IConverter
{
	static constexpr char const* Name = REDName;
	
	sol::object ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua) override
	{
		return make_object(aLua, *static_cast<T*>(aResult.value));
	}

	RED4ext::REDreverse::CScriptableStackFrame::CStackType ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator) override
	{
		RED4ext::REDreverse::CScriptableStackFrame::CStackType result;
		result.type = m_pRtti;
		result.value = apAllocator->New<T>(aObject.as<T>());

		return result;
	}

	bool Is(RED4ext::REDreverse::CRTTIBaseType* apRtti) const override
	{
		if (!Resolve())
			return false;
		
		return apRtti == m_pRtti;
	}

private:

	bool Resolve() const
	{
		if (m_pRtti)
			return true;

		auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
		m_pRtti = pRtti->GetType(RED4ext::FNV1a(Name));

		return m_pRtti != nullptr;
	}

	mutable RED4ext::REDreverse::CRTTIBaseType* m_pRtti{nullptr};
};
