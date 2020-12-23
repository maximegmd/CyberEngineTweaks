#pragma once

#include <sol/sol.hpp>
#include "RED4ext/REDreverse/Scripting/StackFrame.hpp"

namespace TiltedPhoques {
	struct Allocator;
}

namespace Converter
{
	sol::object ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua);
	RED4ext::REDreverse::CScriptableStackFrame::CStackType ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator);
}
