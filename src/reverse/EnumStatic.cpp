#include <stdafx.h>

#include "EnumStatic.h"
#include "Enum.h"

#include "scripting/Scripting.h"

EnumStatic::EnumStatic(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                                       RED4ext::CBaseRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

EnumStatic::~EnumStatic() = default;

sol::object EnumStatic::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    auto result = Type::Index_Impl(acName, aThisEnv);

    if (result != sol::nil)
        return result;

    auto* pEnum = static_cast<RED4ext::CEnum*>(m_pType);

    if (!pEnum)
        return sol::nil;

    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    return Type::NewIndex_Impl(acName, std::move(sol::make_object(luaState, Enum(pEnum, acName))));
}
