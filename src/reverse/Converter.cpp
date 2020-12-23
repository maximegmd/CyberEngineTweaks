#include "Converter.h"

#include "BasicTypes.h"
#include "LuaRED.h"

template<size_t i>
Converter::IConverter* RecursiveFind(MetaArrayImpl<i>& arr, RED4ext::REDreverse::CRTTIBaseType* apRtti)
{
    return nullptr;
}

template<size_t i, typename HeadItem, typename... TailItems>
Converter::IConverter* RecursiveFind(MetaArrayImpl<i, HeadItem, TailItems...>& arr, RED4ext::REDreverse::CRTTIBaseType* apRtti)
{
    if (arr.MetaArrayEntry<i, HeadItem>::value.Is(apRtti))
        return &arr.MetaArrayEntry<i, HeadItem>::value;

    return RecursiveFind<i + 1>(arr, apRtti);
}

template<typename HeadItem, typename... TailItems>
auto Find(MetaArray<HeadItem, TailItems...>& arr, RED4ext::REDreverse::CRTTIBaseType* apRtti)
{
    return RecursiveFind<0, HeadItem, TailItems...>(arr, apRtti);
}

static MetaArray<
    LuaRED<int32_t, "Int32">,
    LuaRED<float, "Float">,
    LuaRED<bool, "Bool">,
    LuaRED<Quaternion, "Quaternion">,
    LuaRED<ItemID, "gameItemID">,
    LuaRED<TweakDBID, "TweakDBID">,
    LuaRED<CName, "CName">
> s_convertersMeta;

sol::object Converter::ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    auto* pConverter = Find(s_convertersMeta, aResult.type);
    if (pConverter)
        pConverter->ToLua(aResult, aLua);

    return sol::nil;
}

RED4ext::REDreverse::CScriptableStackFrame::CStackType Converter::ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    auto* pConverter = Find(s_convertersMeta, apRtti);
    if (pConverter)
        pConverter->ToRED(aObject, apRtti, apAllocator);

    return RED4ext::REDreverse::CScriptableStackFrame::CStackType();
}
