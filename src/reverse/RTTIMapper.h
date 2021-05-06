#pragma once

#include "ClassStatic.h"

struct RTTIMapper
{
    using LockableState = TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref;

    RTTIMapper(const LockableState& acLua, const std::string& acGlobal);
    ~RTTIMapper();

    void Register();

private:

    void RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal);
    void RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti);
    void RegisterScriptAliases(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti);
    void RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal);

    template <class T>
    void ExtendUsertype(const std::string acTypeName, sol::state& aLuaState, sol::table& aLuaGlobal);

    LockableState m_lua;
    std::string m_global;
};
