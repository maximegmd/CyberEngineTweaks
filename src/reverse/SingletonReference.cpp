#include <stdafx.h>

#ifdef __APPLE__
#include "MacShims.hpp"
#endif

#include "SingletonReference.h"

SingletonReference::SingletonReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::CBaseRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

SingletonReference::~SingletonReference() = default;

RED4ext::ScriptInstance SingletonReference::GetHandle() const
{
#ifdef __APPLE__
    auto* pGameInstance = CetMac::GameInstance();
    if (!pGameInstance)
        return {};
#else
    const auto* engine = RED4ext::CGameEngine::Get();
    auto* pGameInstance = engine->framework->gameInstance;
#endif

    return pGameInstance->GetSystem(m_pType);
}
