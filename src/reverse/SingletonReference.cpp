#include <stdafx.h>

#include "SingletonReference.h"

SingletonReference::SingletonReference(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::rtti::IType* apClass)
    : ClassType(aView, apClass)
{
}

SingletonReference::~SingletonReference() = default;

void* SingletonReference::GetHandle() const
{
    const auto* engine = RED4ext::CGameEngine::Get();
    auto* pGameInstance = engine->framework->gameInstance;

    return pGameInstance->GetSystem(m_pType);
}
