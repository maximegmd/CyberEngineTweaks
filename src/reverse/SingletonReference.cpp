#include <stdafx.h>

#include "SingletonReference.h"

SingletonReference::SingletonReference(sol::state_view aView, RED4ext::REDreverse::CClass* apClass)
    : Type(std::move(aView), apClass)
{
}

SingletonReference::~SingletonReference() = default;

RED4ext::REDreverse::Scripting::IScriptable* SingletonReference::GetHandle()
{
    auto* engine = RED4ext::REDreverse::CGameEngine::Get();
    auto* pGameInstance = engine->framework->gameInstance;

    return pGameInstance->GetTypeInstance(m_pType);
}


