#include "stdafx.h"

#include "Converter.h"

auto s_metaVisitor =
    [](auto... args)
{
    return [=](auto&& f) mutable
    {
        f(args...);
    };
}(LuaRED<int8_t, "Int8">(), LuaRED<int16_t, "Int16">(), LuaRED<int32_t, "Int32">(), LuaRED<int64_t, "Int64">(), LuaRED<uint8_t, "Uint8">(), LuaRED<uint16_t, "Uint16">(),
  LuaRED<uint32_t, "Uint32">(), LuaRED<uint64_t, "Uint64">(), LuaRED<float, "Float">(), LuaRED<double, "Double">(), LuaRED<bool, "Bool">(), LuaRED<Quaternion, "Quaternion">(),
  LuaRED<Vector3, "Vector3">(), LuaRED<Vector4, "Vector4">(), LuaRED<EulerAngles, "EulerAngles">(), LuaRED<ItemID, "gameItemID">(), LuaRED<Variant, "Variant">(),
  LuaRED<CRUID, "CRUID">(), LuaRED<gamedataLocKeyWrapper, "gamedataLocKeyWrapper">(), CNameConverter(), TweakDBIDConverter(), EnumConverter(), ClassConverter(),
  RawConverter() // Should always be last resort
);

size_t Converter::Size(RED4ext::CBaseRTTIType* apRtti)
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

sol::object Converter::ToLua(RED4ext::CStackType& aResult, TiltedPhoques::Locked<sol::state, std::recursive_mutex>& aLua)
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

RED4ext::CStackType Converter::ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    RED4ext::CStackType r;
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

void Converter::ToRED(sol::object aObject, RED4ext::CStackType* apType)
{
    auto initStackType = [&](auto& x)
    {
        if (x.Is(apType->type))
        {
            x.ToRED(aObject, apType);
            return true;
        }
        return false;
    };

    auto f = [initStackType](auto&&... xs)
    {
        (... && !initStackType(xs));
    };

    s_metaVisitor(f);
}
