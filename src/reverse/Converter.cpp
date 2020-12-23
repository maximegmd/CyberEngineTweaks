#include "Converter.h"

#include "BasicTypes.h"
#include "LuaRED.h"

template<int64_t i, typename T, std::enable_if_t<i < 0>* = nullptr>
sol::object RecursiveInternalToLua(T& arr, RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    return sol::nil;
}

template<int64_t i, typename T, std::enable_if_t<i >= 0>* = nullptr>
sol::object RecursiveInternalToLua(T& arr, RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    if (std::get<i>(arr).Is(aResult.type))
        return std::get<i>(arr).ToLua(aResult, aLua);

    return RecursiveInternalToLua<i - 1>(arr, aResult, aLua);
}

template<typename T>
sol::object InternalToLua(T& arr, RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    return RecursiveInternalToLua<std::tuple_size<T>::value - 1>(arr, aResult, aLua);
}

template<int64_t i, typename T, std::enable_if_t<i < 0>* = nullptr>
RED4ext::REDreverse::CScriptableStackFrame::CStackType RecursiveInternalToRED(T& arr, sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    return {};
}

template<int64_t i, typename T, std::enable_if_t<i >= 0>* = nullptr>
RED4ext::REDreverse::CScriptableStackFrame::CStackType RecursiveInternalToRED(T& arr, sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    if (std::get<i>(arr).Is(apRtti))
        return std::get<i>(arr).ToRED(aObject, apRtti, apAllocator);

    return RecursiveInternalToRED<i - 1>(arr, aObject, apRtti, apAllocator);
}

template<typename T>
RED4ext::REDreverse::CScriptableStackFrame::CStackType InternalToRED(T& arr, sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    return RecursiveInternalToRED<std::tuple_size<T>::value - 1>(arr, aObject, apRtti, apAllocator);
}

static std::tuple<
    LuaRED<int8_t, "Int8">,
    LuaRED<int16_t, "Int16">,
    LuaRED<int32_t, "Int32">,
    LuaRED<int64_t, "Int64">,
    LuaRED<uint8_t, "Uint8">,
    LuaRED<uint16_t, "Uint16">,
    LuaRED<uint32_t, "Uint32">,
    LuaRED<uint64_t, "Uint64">,
    LuaRED<float, "Float">,
    LuaRED<bool, "Bool">,
    LuaRED<Quaternion, "Quaternion">,
    LuaRED<Vector4, "Vector4">,
    LuaRED<EulerAngles, "EulerAngles">,
    LuaRED<ItemID, "gameItemID">,
    LuaRED<TweakDBID, "TweakDBID">,
    LuaRED<CName, "CName">
> s_convertersMeta;

sol::object Converter::ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    return InternalToLua(s_convertersMeta, aResult, aLua);
}

RED4ext::REDreverse::CScriptableStackFrame::CStackType Converter::ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    return InternalToRED(s_convertersMeta, aObject, apRtti, apAllocator);
}
