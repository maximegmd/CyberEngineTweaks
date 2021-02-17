#pragma once

#include <reverse/BasicTypes.h>

struct TweakDB
{
    TweakDB(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aLua);

    void DebugStats();
    sol::object GetRecord(TweakDBID aDBID);
    sol::object Query(TweakDBID aDBID);
    sol::object GetFlat(TweakDBID aDBID);
    bool SetFlat(TweakDBID aDBID, sol::object aValue);
    bool UpdateRecordByID(TweakDBID aDBID);
    bool UpdateRecord(sol::object aValue);

private:
    TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref m_lua;
};