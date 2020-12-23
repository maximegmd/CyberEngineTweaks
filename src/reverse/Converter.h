#pragma once
#include <sol/sol.hpp>
#include "RED4ext/REDreverse/Scripting/StackFrame.hpp"

namespace TiltedPhoques {
	struct Allocator;
}

namespace Converter
{
	struct IConverter
	{
		virtual sol::object ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua) = 0;
		virtual RED4ext::REDreverse::CScriptableStackFrame::CStackType ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator) = 0;

		virtual bool Is(RED4ext::REDreverse::CRTTIBaseType* apRtti) const = 0;
	};
	
	sol::object ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua);
	RED4ext::REDreverse::CScriptableStackFrame::CStackType ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator);
}
