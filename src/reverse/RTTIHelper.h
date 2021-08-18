#pragma once

#include "BasicTypes.h"

struct RTTIHelper
{
    using LockableState = TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref;
    using RedFunctionMap = TiltedPhoques::Map<uint64_t, TiltedPhoques::Map<uint64_t, RED4ext::CBaseFunction*>>;
    using LuaFunctionMap = TiltedPhoques::Map<uint64_t, TiltedPhoques::Map<uint64_t, sol::function>>;

    ~RTTIHelper() = default;

    void AddFunctionAlias(const std::string& acAliasFuncName, const std::string& acOrigClassName, const std::string& acOrigFuncName);
    void AddFunctionAlias(const std::string& acAliasClassName, const std::string& acAliasFuncName,
                          const std::string& acOrigClassName, const std::string& acOrigFuncName);

    sol::function ResolveFunction(const std::string& acFuncName);
    sol::function ResolveFunction(RED4ext::CClass* apClass, const std::string& acFuncName, bool aIsMember);
    
    RED4ext::ScriptInstance ResolveHandle(RED4ext::CBaseFunction* apFunc, sol::variadic_args& aArgs, uint64_t& aArgOffset) const;
    sol::variadic_results ExecuteFunction(RED4ext::CBaseFunction* apFunc, RED4ext::ScriptInstance apHandle,
                                          sol::variadic_args aLuaArgs, uint64_t aLuaArgOffset,
                                          std::string& aErrorMessage) const;
    
    RED4ext::ScriptInstance NewInstance(RED4ext::IRTTIType* apType, sol::optional<sol::table> aProps, TiltedPhoques::Allocator* apAllocator) const;
    sol::object NewInstance(RED4ext::IRTTIType* apType, sol::optional<sol::table> aProps) const;
    sol::object NewHandle(RED4ext::IRTTIType* apType, sol::optional<sol::table> aProps) const;

    sol::object GetProperty(RED4ext::CClass* apClass, RED4ext::ScriptInstance apHandle, const std::string& acPropName, bool& aSuccess) const;
    void SetProperty(RED4ext::CClass* apClass, RED4ext::ScriptInstance apHandle, const std::string& acPropName, sol::object aPropValue, bool& aSuccess) const;

    static void Initialize(const LockableState& acLua);
    static void Shutdown();
    static RTTIHelper& Get();

private:

    RTTIHelper(const LockableState& acLua);

    void InitializeRTTI();
    void ParseGlobalStatics();

    sol::function GetResolvedFunction(const uint64_t acFuncHash) const;
    sol::function GetResolvedFunction(const uint64_t acClassHash, const uint64_t acFuncHash, bool aIsMember) const;
    void AddResolvedFunction(const uint64_t acFuncHash, sol::function& acFunc);
    void AddResolvedFunction(const uint64_t acClassHash, const uint64_t acFuncHash, sol::function& acFunc, bool aIsMember);

    RED4ext::CBaseFunction* FindFunction(const uint64_t acFullNameHash) const;
    RED4ext::CBaseFunction* FindFunction(RED4ext::CClass* apClass, const uint64_t acFullNameHash) const;
    std::map<uint64_t, RED4ext::CBaseFunction*> FindFunctions(const uint64_t acShortNameHash) const;
    std::map<uint64_t, RED4ext::CBaseFunction*> FindFunctions(RED4ext::CClass* apClass, const uint64_t acShortNameHash, bool aIsMember) const;
    
    sol::function MakeInvokableFunction(RED4ext::CBaseFunction* apFunc);
    sol::function MakeInvokableOverload(std::map<uint64_t, RED4ext::CBaseFunction*> aOverloadedFuncs);

    RED4ext::ScriptInstance NewPlaceholder(RED4ext::IRTTIType* apType, TiltedPhoques::Allocator* apAllocator) const;
    bool IsClassReferenceType(RED4ext::CClass* apClass) const;
    void FreeInstance(RED4ext::CStackType& aStackType, bool aOwnValue, bool aNewValue, TiltedPhoques::Allocator* apAllocator) const;
    void FreeInstance(RED4ext::IRTTIType* apType, void* apValue, bool aOwnValue, bool aNewValue, TiltedPhoques::Allocator* apAllocator) const;

    enum
    {
        kGlobalHash = 0,
        kStaticScope = 0,
        kMemberScope = 1,
        kTrackedOverloadCalls = 256,
    };

    struct Overload
    {
        RED4ext::CBaseFunction* func;
        std::string lastError;
        uint32_t totalCalls;
    };

    LockableState m_lua;
    RED4ext::CRTTISystem* m_pRtti;
    RED4ext::CClass* m_pGameInstanceType;
    ScriptGameInstance* m_pGameInstance;
    RED4ext::ScriptInstance m_pPlayerSystem;
    RedFunctionMap m_extendedFunctions;
    LuaFunctionMap m_resolvedFunctions[2];
};
