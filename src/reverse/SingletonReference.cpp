#include "SingletonReference.h"
#include <RED4ext/REDreverse/Scripting/IScriptable.hpp>

#include "RED4ext/REDreverse/GameEngine.hpp"
#include "RED4ext/REDreverse/RTTI/RTTISystem.hpp"

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


