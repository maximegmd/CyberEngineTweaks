#pragma once

struct LuaSandbox;
struct RTTIMapper
{
    using LockableState = TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref;

    RTTIMapper(const LockableState& acpLua, LuaSandbox& apSandbox);
    ~RTTIMapper();

    void Register();
    void Refresh();

    static void SanitizeName(std::string& aName);
    static red3lib::CName TryResolveTypeName(sol::object aValue);

private:

    void RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal) const;
    void RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti);
    void RegisterDirectGlobals(sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti);
    void RegisterScriptAliases(sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti);
    void RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal) const;

    LockableState m_lua;
    LuaSandbox& m_sandbox;
};
