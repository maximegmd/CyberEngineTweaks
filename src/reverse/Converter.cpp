#include "Converter.h"

#include "BasicTypes.h"
#include "LuaRED.h"

auto s_metaVisitor = [](auto... args) {
    return [=](auto&& f) mutable { f(args...); };
}(
    LuaRED<int8_t, "Int8">(),
    LuaRED<int16_t, "Int16">(),
    LuaRED<int32_t, "Int32">(),
    LuaRED<int64_t, "Int64">(),
    LuaRED<uint8_t, "Uint8">(),
    LuaRED<uint16_t, "Uint16">(),
    LuaRED<uint32_t, "Uint32">(),
    LuaRED<uint64_t, "Uint64">(),
    LuaRED<float, "Float">(),
    LuaRED<bool, "Bool">(),
    LuaRED<Quaternion, "Quaternion">(),
    LuaRED<Vector3, "Vector3">(),
    LuaRED<Vector4, "Vector4">(),
    LuaRED<EulerAngles, "EulerAngles">(),
    LuaRED<ItemID, "gameItemID">(),
    LuaRED<TweakDBID, "TweakDBID">(),
    LuaRED<CName, "CName">()
    );

size_t Converter::Size(RED4ext::REDreverse::CRTTIBaseType* apRtti)
{
    size_t s = 0;

    auto initSize = [&](auto& x)
    {
        if (x.Is(apRtti))
        {
            s = x.Size();
            return true;
        }
        return false;
    };

    auto f = [initSize](auto&&... xs)
    {
        (... && !initSize(xs));
    };

    s_metaVisitor(f);

    return s;
}

sol::object Converter::ToLua(RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult, sol::state_view aLua)
{
    sol::object o = sol::nil;
    auto initLuaObject = [&](auto& x)
    {
        if (x.Is(aResult.type))
        {
            o = x.ToLua(aResult, aLua);
            return true;
        }
        return false;
    };

    auto f = [initLuaObject](auto&&... xs)
    {
        (... && !initLuaObject(xs));
    };

    s_metaVisitor(f);

    return o;
}

RED4ext::REDreverse::CScriptableStackFrame::CStackType Converter::ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    RED4ext::REDreverse::CScriptableStackFrame::CStackType r;
    auto initStackType = [&](auto& x)
    {
        if (x.Is(apRtti))
        {
            r = x.ToRED(aObject, apRtti, apAllocator);
            return true;
        }
        return false;
    };

    auto f = [initStackType](auto&&... xs)
    {
        (... && !initStackType(xs));
    };

    s_metaVisitor(f);

    return r;
}
