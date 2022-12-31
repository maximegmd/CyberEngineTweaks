#include "stdafx.h"

#include "ClassStatic.h"

#include "RTTIHelper.h"
#include "StrongReference.h"
#include "Utils.h"

ClassStatic::ClassStatic(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::CBaseRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

ClassStatic::~ClassStatic() = default;

sol::function ClassStatic::GetFactory()
{
    if (!m_factory)
    {
        auto lockedState = m_lua.Lock();
        auto& luaState = lockedState.Get();

        m_factory = MakeSolFunction(luaState, [this](sol::optional<sol::table> aProps) { return RTTIHelper::Get().NewHandle(m_pType, aProps); });
    }

    return m_factory;
}
