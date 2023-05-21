#pragma once

#include "BasicTypes.h"
#include "WeakReference.h"

#include <RED4ext/Scripting/IScriptable.hpp>
#include <RED4ext/RTTITypes.hpp>

struct NativeProxyType : RED4ext::CClass
{
    NativeProxyType(RED4ext::CClass* apBase);

    RED4ext::Handle<RED4ext::IScriptable> CreateHandle();

    const bool IsEqual(const RED4ext::ScriptInstance aLhs, const RED4ext::ScriptInstance aRhs, uint32_t a3) final; // 48
    void Assign(RED4ext::ScriptInstance aLhs, const RED4ext::ScriptInstance aRhs) const final; // 50
    void ConstructCls(RED4ext::ScriptInstance aMemory) const final; // D8
    void DestructCls(RED4ext::ScriptInstance aMemory) const final; // E0
    void* AllocMemory() const final; // E8

    static NativeProxyType* ResolveProxy(const std::string& aName);
    static RED4ext::CName AddSignature(NativeProxyType* apProxy, const std::string& aName, const sol::table& aArgs);
    static std::string FormatTypeName(RED4ext::CRTTISystem* apRtti, RED4ext::CBaseRTTIType* apType);
};

struct NativeProxy
{
    using LockableState = TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref;

    NativeProxy(NativeProxy::LockableState aLua, sol::environment aEnvironment, const sol::object& aSpec);
    NativeProxy(NativeProxy::LockableState aLua, sol::environment aEnvironment, const std::string& aInterface, const sol::object& aSpec);
    NativeProxy(NativeProxy&& aOther) noexcept;

    [[nodiscard]] WeakReference GetTarget(/*sol::this_state aState*/) const;
    [[nodiscard]] CName GetFunction(const std::string& aName) const;
    [[nodiscard]] CName GetDefaultFunction() const;

    bool AddFunction(NativeProxyType* apProxy, const std::string& aName, const sol::object& aSpec);

    static void Callback(RED4ext::IScriptable* apSelf, RED4ext::CStackFrame* apFrame, void* apOut, int64_t);

    RED4ext::Handle<RED4ext::IScriptable> m_target;
    TiltedPhoques::Map<RED4ext::CName, sol::protected_function> m_functions;
    TiltedPhoques::Map<RED4ext::CName, RED4ext::CName> m_signatures;
    sol::environment m_environment;
    LockableState m_lua;
};
