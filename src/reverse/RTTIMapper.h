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
    static RED4ext::CName TryResolveTypeName(sol::object aValue);

private:
    struct FuncFlags
    {
        uint32_t isNative : 1;        // 00
        uint32_t isStatic : 1;        // 01
        uint32_t isFinal : 1;         // 02
        uint32_t isEvent : 1;         // 03
        uint32_t isExec : 1;          // 04
        uint32_t isUndefinedBody : 1; // 05 Unconfirmed (unset for scripted funcs without body)
        uint32_t isTimer : 1;         // 06 Unconfirmed
        uint32_t isPrivate : 1;       // 07
        uint32_t isProtected : 1;     // 08
        uint32_t isPublic : 1;        // 09
        uint32_t b11 : 1;             // 0A
        uint32_t b12 : 1;             // 0B
        uint32_t b13 : 1;             // 0C
        uint32_t isConst : 1;         // 0D
        uint32_t isQuest : 1;         // 0E
        uint32_t isThreadsafe : 1;    // 0F
        uint32_t b16 : 16;
    };
    RED4EXT_ASSERT_SIZE(FuncFlags, 0x4);

    void RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal) const;
    void RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti);
    void RegisterDirectGlobals(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti);
    void RegisterScriptAliases(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti);
    void RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal) const;

    template <class T> void ExtendUsertype(const std::string acTypeName, sol::state& aLuaState, sol::table& aLuaGlobal) const;

    LockableState m_lua;
    LuaSandbox& m_sandbox;
};
