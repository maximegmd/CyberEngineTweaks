#pragma once

#include <reverse/BasicTypes.h>

struct TweakDB
{
    TweakDB(const Lockable<sol::state*, std::recursive_mutex>& aLua);

    void DebugStats();
    sol::object GetRecord(TweakDBID aDBID);
    sol::object Query(TweakDBID aDBID);
    sol::object GetFlat(TweakDBID aDBID);
    bool SetFlat(TweakDBID aDBID, sol::object aValue);
    bool UpdateRecordByID(TweakDBID aDBID);
    bool UpdateRecord(sol::object aValue);

private:
    Lockable<sol::state*, std::recursive_mutex> m_lua;
};