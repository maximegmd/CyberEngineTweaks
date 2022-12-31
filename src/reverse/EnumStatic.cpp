#include "stdafx.h"

#include "EnumStatic.h"
#include "Enum.h"

#include "scripting/Scripting.h"

EnumStatic::EnumStatic(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::CBaseRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

EnumStatic::~EnumStatic() = default;

sol::object EnumStatic::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    // TODO - this should not use Type:: probably but ClassType
    auto result = Type::Index_Impl(acName, aThisEnv);

    if (result != sol::nil)
        return result;

    const auto* cpEnum = static_cast<RED4ext::CEnum*>(m_pType);

    if (!cpEnum)
        return sol::nil;

    auto lockedState = m_lua.Lock();
    const auto& cLuaState = lockedState.Get();

    // TODO - this should not use Type:: probably but ClassType
    return Type::NewIndex_Impl(acName, make_object(cLuaState, Enum(cpEnum, acName)));
}
