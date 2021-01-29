#include <stdafx.h>

#include <RED4ext/Types/TweakDB.hpp>

#include <scripting/Scripting.h>

#include "TweakDB.h"

TweakDB::TweakDB(sol::state_view aLua)
    : m_lua(std::move(aLua))
{
}

sol::object TweakDB::GetRecord(TweakDBID aDBID)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    auto* record = pTDB->GetRecord(dbid);
    if (record == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType;
    stackType.type = (*record)->GetType();
    stackType.value = record;
    return Scripting::ToLua(m_lua, stackType);
}

sol::object TweakDB::Query(TweakDBID aDBID)
{
    static auto* pRTTI = RED4ext::CRTTISystem::Get();
    static auto* pArrayTweakDBIDType = pRTTI->GetType("array:TweakDBID");
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    auto* queryResult = pTDB->Query(dbid);
    if (queryResult == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType;
    stackType.type = pArrayTweakDBIDType;
    stackType.value = queryResult;
    return Scripting::ToLua(m_lua, stackType);
}

sol::object TweakDB::GetFlat(TweakDBID aDBID)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    auto* flatValue = pTDB->GetFlatValue(dbid);
    if (flatValue == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType;
    flatValue->GetValue(&stackType);
    return Scripting::ToLua(m_lua, stackType);
}

bool TweakDB::SetFlat(TweakDBID aDBID, sol::object aValue)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    auto* flatValue = pTDB->GetFlatValue(dbid);
    if (flatValue == nullptr)
        return false;

    RED4ext::CStackType stackTypeCurrent;
    flatValue->GetValue(&stackTypeCurrent);
    bool isDefaultValue = false;
    pTDB->defaultValueByType.for_each([&stackTypeCurrent, &isDefaultValue](const RED4ext::CName&, RED4ext::TweakDB::FlatValue* defaultFlatValue)
        {
            if (stackTypeCurrent.value == defaultFlatValue)
                isDefaultValue = true;
        });
    if (isDefaultValue) return false; // catastrophic. don't modify.

    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1024);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    RED4ext::CStackType stackType = Scripting::ToRED(aValue, stackTypeCurrent.type, &s_scratchMemory);
    return flatValue->SetValue(stackType);
}
