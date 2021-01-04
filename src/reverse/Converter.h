#pragma once

namespace TiltedPhoques {
    struct Allocator;
}

namespace Converter
{
    size_t Size(RED4ext::IRTTIType* apRtti);
    sol::object ToLua(RED4ext::CStackType& aResult, sol::state_view aLua);
    RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);
}
