#include <stdafx.h>

#include "SingletonReference.h"

SingletonReference::SingletonReference(sol::state_view aView, RED4ext::IRTTIType* apClass)
    : ClassType(std::move(aView), apClass)
{
}

SingletonReference::~SingletonReference() = default;

RED4ext::ScriptInstance SingletonReference::GetHandle()
{
    auto* engine = RED4ext::CGameEngine::Get();
    auto* pGameInstance = engine->framework->gameInstance;

    return pGameInstance->GetInstance(m_pType);
}


