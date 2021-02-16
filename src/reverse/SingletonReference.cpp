#include <stdafx.h>

#include "SingletonReference.h"

SingletonReference::SingletonReference(const Lockable<sol::state_view, std::recursive_mutex>& aView,
                                       RED4ext::IRTTIType* apClass)
    : ClassType(aView, apClass)
{
}

SingletonReference::~SingletonReference() = default;

RED4ext::ScriptInstance SingletonReference::GetHandle()
{
    auto* engine = RED4ext::CGameEngine::Get();
    auto* pGameInstance = engine->framework->gameInstance;

    return pGameInstance->GetInstance(m_pType);
}


