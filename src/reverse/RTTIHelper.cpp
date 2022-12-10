#include <stdafx.h>

#include "RTTIHelper.h"

#include "ClassReference.h"
#include "StrongReference.h"
#include "WeakReference.h"

#include <common/ScopeGuard.h>
#include <scripting/Scripting.h>
#include <Utils.h>

namespace
{
constexpr bool s_cEnableOverloads = true;
constexpr bool s_cLogAllOverloadVariants = true;
constexpr bool s_cThrowLuaErrors = true;

std::unique_ptr<RTTIHelper> s_pInstance{nullptr};

using TCallScriptFunction = bool (*)(RED4ext::IFunction* apFunction, RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, void* apResult, void* apResultType);

RED4ext::RelocFunc<TCallScriptFunction> CallScriptFunction(RED4ext::Addresses::CBaseFunction_InternalExecute);
} // namespace

void RTTIHelper::Initialize(const LockableState& acLua, LuaSandbox& apSandbox)
{
    if (!s_pInstance)
    {
        s_pInstance.reset(new RTTIHelper(acLua, apSandbox));
    }
}

void RTTIHelper::PostInitialize()
{
    if (s_pInstance)
    {
        s_pInstance->InitializeRuntime();
    }
}

void RTTIHelper::Shutdown()
{
    // Since m_resolvedFunctions contains references to Lua state
    // it's important to call destructor while Lua state exists.
    // Otherwise sol will throw invalid reference exception.
    s_pInstance.reset(nullptr);
}

RTTIHelper& RTTIHelper::Get()
{
    return *s_pInstance;
}

RTTIHelper::RTTIHelper(const LockableState& acpLua, LuaSandbox& apSandbox)
    : m_lua(acpLua)
    , m_sandbox(apSandbox)
{
    InitializeRTTI();
    ParseGlobalStatics();
}

void RTTIHelper::InitializeRTTI()
{
    m_pRtti = RED4ext::CRTTISystem::Get();
    m_pGameInstanceType = m_pRtti->GetClass(RED4ext::FNV1a64("ScriptGameInstance"));
}

void RTTIHelper::InitializeRuntime()
{
    const auto cpEngine = RED4ext::CGameEngine::Get();
    const auto cpGameInstance = cpEngine->framework->gameInstance;
    const auto cpPlayerSystemType = m_pRtti->GetType(RED4ext::FNV1a64("cpPlayerSystem"));

    m_pGameInstance = static_cast<ScriptGameInstance*>(m_pGameInstanceType->CreateInstance());
    m_pGameInstance->gameInstance = cpGameInstance;

    m_pPlayerSystem = cpGameInstance->GetInstance(cpPlayerSystemType);
}

void RTTIHelper::ParseGlobalStatics()
{
    m_pRtti->funcs.for_each(
        [this](RED4ext::CName aOrigName, RED4ext::CGlobalFunction* apFunc)
        {
            const std::string cOrigName = aOrigName.ToString();
            const auto cClassSep = cOrigName.find("::");

            if (cClassSep != std::string::npos)
            {
                const auto cClassName = cOrigName.substr(0, cClassSep);
                const auto cFullName = cOrigName.substr(cClassSep + 2);

                const auto cClassHash = RED4ext::FNV1a64(cClassName.c_str());
                const auto cFullHash = RED4ext::FNV1a64(cFullName.c_str());

                if (!m_extendedFunctions.contains(cClassHash))
                    m_extendedFunctions.emplace(cClassHash, 0);

                m_extendedFunctions.at(cClassHash).emplace(cFullHash, apFunc);
            }
        });
}

void RTTIHelper::AddFunctionAlias(const std::string&, const std::string& acOrigClassName, const std::string& acOrigFuncName)
{
    auto* pClass = m_pRtti->GetClass(RED4ext::FNV1a64(acOrigClassName.c_str()));
    if (pClass)
    {
        auto* pFunc = FindFunction(pClass, RED4ext::FNV1a64(acOrigFuncName.c_str()));

        if (pFunc)
        {
            if (!m_extendedFunctions.contains(kGlobalHash))
                m_extendedFunctions.emplace(kGlobalHash, 0);

            m_extendedFunctions.at(kGlobalHash).emplace(pFunc->fullName.hash, pFunc);
        }
    }
}

void RTTIHelper::AddFunctionAlias(const std::string& acAliasClassName, const std::string&, const std::string& acOrigClassName, const std::string& acOrigFuncName)
{
    auto* pClass = m_pRtti->GetClass(RED4ext::FNV1a64(acOrigClassName.c_str()));
    if (pClass)
    {
        auto* pFunc = FindFunction(pClass, RED4ext::FNV1a64(acOrigFuncName.c_str()));

        if (pFunc)
        {
            const auto cClassHash = RED4ext::FNV1a64(acAliasClassName.c_str());

            if (!m_extendedFunctions.contains(cClassHash))
                m_extendedFunctions.emplace(cClassHash, 0);

            m_extendedFunctions.at(cClassHash).emplace(pFunc->fullName.hash, pFunc);
        }
    }
}

bool RTTIHelper::IsFunctionAlias(RED4ext::CBaseFunction* apFunc)
{
    static constexpr auto s_cTweakDBInterfaceHash = RED4ext::FNV1a64("gamedataTweakDBInterface");
    static constexpr auto s_cTDBIDHelperHash = RED4ext::FNV1a64("gamedataTDBIDHelper");

    if (m_extendedFunctions.contains(kGlobalHash))
    {
        const auto& extendedFuncs = m_extendedFunctions.at(kGlobalHash);

        if (extendedFuncs.contains(apFunc->fullName.hash))
            return true;
    }

    if (apFunc->GetParent())
    {
        const auto cClassHash = apFunc->GetParent()->name.hash;

        // TweakDBInterface and TDBID classes are special.
        // All of their methods are non-static, but they can only be used as static ones.
        if (cClassHash == s_cTweakDBInterfaceHash || cClassHash == s_cTDBIDHelperHash)
            return true;

        if (m_extendedFunctions.contains(cClassHash))
        {
            const auto& extendedFuncs = m_extendedFunctions.at(cClassHash);

            if (extendedFuncs.contains(apFunc->fullName.hash))
                return true;
        }
    }

    return false;
}

sol::function RTTIHelper::GetResolvedFunction(const uint64_t acFuncHash) const
{
    return GetResolvedFunction(kGlobalHash, acFuncHash, false);
}

sol::function RTTIHelper::GetResolvedFunction(const uint64_t acClassHash, const uint64_t acFuncHash, bool aIsMember) const
{
    const auto cScope = aIsMember ? kMemberScope : kStaticScope;

    if (m_resolvedFunctions[cScope].contains(acClassHash))
    {
        auto& classFuncs = m_resolvedFunctions[cScope].at(acClassHash);

        if (classFuncs.contains(acFuncHash))
            return classFuncs.at(acFuncHash);
    }

    return sol::nil;
}

// Add global function to resolved map
void RTTIHelper::AddResolvedFunction(const uint64_t acFuncHash, sol::function& acFunc)
{
    AddResolvedFunction(kGlobalHash, acFuncHash, acFunc, false);
}

// Add class function to resolved map
void RTTIHelper::AddResolvedFunction(const uint64_t acClassHash, const uint64_t acFuncHash, sol::function& acFunc, bool aIsMember)
{
    const auto cScope = aIsMember ? kMemberScope : kStaticScope;

    if (!m_resolvedFunctions[cScope].contains(acClassHash))
        m_resolvedFunctions[cScope].emplace(acClassHash, 0);

    m_resolvedFunctions[cScope].at(acClassHash).emplace(acFuncHash, acFunc);
}

// Find global function matching full name
RED4ext::CBaseFunction* RTTIHelper::FindFunction(const uint64_t acFullNameHash) const
{
    RED4ext::CBaseFunction* pFunc = m_pRtti->GetFunction(acFullNameHash);

    if (!pFunc && m_extendedFunctions.contains(kGlobalHash))
    {
        auto& extendedFuncs = m_extendedFunctions.at(kGlobalHash);

        if (extendedFuncs.contains(acFullNameHash))
            pFunc = extendedFuncs.at(acFullNameHash);
    }

    return pFunc;
}

// Find class function matching full name
RED4ext::CBaseFunction* RTTIHelper::FindFunction(RED4ext::CClass* apClass, const uint64_t acFullNameHash) const
{
    while (apClass != nullptr)
    {
        if (m_extendedFunctions.contains(apClass->name.hash))
        {
            auto& extendedFuncs = m_extendedFunctions.at(apClass->name.hash);

            if (extendedFuncs.contains(acFullNameHash))
                return extendedFuncs.at(acFullNameHash);
        }

        for (uint32_t i = 0; i < apClass->funcs.size; ++i)
        {
            if (apClass->funcs.entries[i]->fullName.hash == acFullNameHash)
                return apClass->funcs.entries[i];
        }

        for (uint32_t i = 0; i < apClass->staticFuncs.size; ++i)
        {
            if (apClass->staticFuncs.entries[i]->fullName.hash == acFullNameHash)
                return apClass->staticFuncs.entries[i];
        }

        apClass = apClass->parent;
    }

    return nullptr;
}

// Find all global functions matching short name
std::map<uint64_t, RED4ext::CBaseFunction*> RTTIHelper::FindFunctions(const uint64_t acShortNameHash) const
{
    std::map<uint64_t, RED4ext::CBaseFunction*> results{};

    m_pRtti->funcs.for_each(
        [&results, &acShortNameHash](RED4ext::CName aFullName, RED4ext::CGlobalFunction* apFunction)
        {
            if (apFunction->shortName.hash == acShortNameHash)
                results.emplace(aFullName.hash, apFunction);
        });

    if (m_extendedFunctions.contains(kGlobalHash))
    {
        auto& extendedFuncs = m_extendedFunctions.at(kGlobalHash);

        for (const auto& cEntry : extendedFuncs)
        {
            if (cEntry.second->shortName.hash == acShortNameHash)
                results.emplace(cEntry.first, cEntry.second);
        }
    }

    return results;
}

// Find all class functions matching short name
std::map<uint64_t, RED4ext::CBaseFunction*> RTTIHelper::FindFunctions(RED4ext::CClass* apClass, const uint64_t acShortNameHash, bool aIsMember) const
{
    std::map<uint64_t, RED4ext::CBaseFunction*> results{};

    while (apClass != nullptr)
    {
        if (aIsMember)
        {
            for (uint32_t i = 0; i < apClass->funcs.size; ++i)
            {
                if (apClass->funcs.entries[i]->shortName.hash == acShortNameHash)
                    results.emplace(apClass->funcs.entries[i]->fullName.hash, apClass->funcs.entries[i]);
            }
        }
        else
        {
            for (uint32_t i = 0; i < apClass->staticFuncs.size; ++i)
            {
                if (apClass->staticFuncs.entries[i]->shortName.hash == acShortNameHash)
                    results.emplace(apClass->staticFuncs.entries[i]->fullName.hash, apClass->staticFuncs.entries[i]);
            }

            if (m_extendedFunctions.contains(apClass->name.hash))
            {
                auto& extendedFuncs = m_extendedFunctions.at(apClass->name.hash);

                for (const auto& cEntry : extendedFuncs)
                {
                    if (cEntry.second->shortName.hash == acShortNameHash)
                        results.emplace(cEntry.first, cEntry.second);
                }
            }
        }

        apClass = apClass->parent;
    }

    return results;
}

// Resolve global function to Lua function
sol::function RTTIHelper::ResolveFunction(const std::string& acFuncName)
{
    if (!m_pRtti)
        return sol::nil;

    const auto cFuncHash = RED4ext::FNV1a64(acFuncName.c_str());

    auto invokable = GetResolvedFunction(cFuncHash);
    if (invokable != sol::nil)
        return invokable;

    const auto cIsFullName = acFuncName.find(';') != std::string::npos;

    if (cIsFullName)
    {
        auto pFunc = FindFunction(cFuncHash);

        if (!pFunc)
        {
            pFunc = FindFunction(m_pGameInstanceType, cFuncHash);

            if (!pFunc)
                return sol::nil;
        }

        invokable = MakeInvokableFunction(pFunc);
        AddResolvedFunction(cFuncHash, invokable);
    }
    else
    {
        auto overloads = FindFunctions(cFuncHash);

        if (overloads.empty())
        {
            overloads = FindFunctions(m_pGameInstanceType, cFuncHash, false);

            if (overloads.empty())
                return sol::nil;
        }

        if (!s_cEnableOverloads || overloads.size() == 1)
            invokable = MakeInvokableFunction(overloads.begin()->second);
        else
            invokable = MakeInvokableOverload(overloads);

        AddResolvedFunction(cFuncHash, invokable);
    }

    return invokable;
}

// Resolve class function to Lua function
sol::function RTTIHelper::ResolveFunction(RED4ext::CClass* apClass, const std::string& acFuncName, bool aIsMember)
{
    if (!m_pRtti)
        return sol::nil;

    if (apClass == nullptr)
        return ResolveFunction(acFuncName);

    const auto cClassHash = RED4ext::FNV1a64(apClass->name.ToString());
    const auto cFuncHash = RED4ext::FNV1a64(acFuncName.c_str());

    auto invokable = GetResolvedFunction(cClassHash, cFuncHash, aIsMember);
    if (invokable != sol::nil)
        return invokable;

    const auto cIsFullName = acFuncName.find(';') != std::string::npos;

    if (cIsFullName)
    {
        const auto pFunc = FindFunction(apClass, cFuncHash);

        if (!pFunc)
            return sol::nil;

        invokable = MakeInvokableFunction(pFunc);
        AddResolvedFunction(cClassHash, cFuncHash, invokable, aIsMember);
    }
    else
    {
        auto overloads = FindFunctions(apClass, cFuncHash, aIsMember);

        if (overloads.empty())
        {
            // Try different scope if nothing was found in the needed one
            // Allows static functions to be called from the instance
            overloads = FindFunctions(apClass, cFuncHash, !aIsMember);

            if (overloads.empty())
                return sol::nil;
        }

        if (!s_cEnableOverloads || overloads.size() == 1)
            invokable = MakeInvokableFunction(overloads.begin()->second);
        else
            invokable = MakeInvokableOverload(overloads);

        AddResolvedFunction(cClassHash, cFuncHash, invokable, aIsMember);
    }

    return invokable;
}

sol::function RTTIHelper::MakeInvokableFunction(RED4ext::CBaseFunction* apFunc)
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    const bool cAllowNull = IsFunctionAlias(apFunc);

    return MakeSolFunction(
        luaState,
        [this, apFunc, cAllowNull](sol::variadic_args aArgs, sol::this_state aState, sol::this_environment aEnv) -> sol::variadic_results
        {
            uint64_t argOffset = 0;
            const auto pHandle = ResolveHandle(apFunc, aArgs, argOffset);

            std::string errorMessage;
            auto result = ExecuteFunction(apFunc, pHandle, aArgs, argOffset, errorMessage, cAllowNull);

            if (!errorMessage.empty())
            {
                if constexpr (s_cThrowLuaErrors)
                {
                    luaL_error(aState, errorMessage.c_str());
                }
                else
                {
                    const sol::environment cEnv = aEnv;
                    auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                    logger->error("Error: {}", errorMessage);
                }
            }

            return result;
        });
}

sol::function RTTIHelper::MakeInvokableOverload(std::map<uint64_t, RED4ext::CBaseFunction*> aOverloadedFuncs) const
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    TiltedPhoques::Vector<Overload> variants;

    for (const auto& func : aOverloadedFuncs | std::views::values)
        variants.emplace_back(func);

    return MakeSolFunction(
        luaState,
        [this, variants](sol::variadic_args aArgs, sol::this_state aState, sol::this_environment aEnv) mutable -> sol::variadic_results
        {
            for (auto variant = variants.begin(); variant != variants.end(); ++variant)
            {
                variant->lastError.clear();

                uint64_t argOffset = 0;
                const auto pHandle = ResolveHandle(variant->func, aArgs, argOffset);

                auto result = ExecuteFunction(variant->func, pHandle, aArgs, argOffset, variant->lastError);

                if (variant->lastError.empty())
                {
                    if (variant->totalCalls < kTrackedOverloadCalls)
                    {
                        ++variant->totalCalls;

                        if (variant != variants.begin())
                        {
                            const auto previous = variant - 1;

                            if (variant->totalCalls > previous->totalCalls)
                                std::iter_swap(previous, variant);
                        }
                    }

                    return result;
                }
            }

            if constexpr (s_cLogAllOverloadVariants)
            {
                const sol::environment cEnv = aEnv;
                const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

                for (auto& variant : variants)
                    logger->info("{}: {}", variant.func->fullName.ToString(), variant.lastError);

                logger->flush();
            }

            std::string errorMessage = fmt::format("No matching overload of '{}' function.", variants.begin()->func->shortName.ToString());

            if constexpr (s_cThrowLuaErrors)
            {
                luaL_error(aState, errorMessage.c_str());
            }
            else
            {
                const sol::environment cEnv = aEnv;
                auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error("Error: {}", errorMessage);
            }

            return {};
        });
}

RED4ext::IScriptable* RTTIHelper::ResolveHandle(RED4ext::CBaseFunction* apFunc, sol::variadic_args& aArgs, uint64_t& aArgOffset) const
{
    // Case 1: obj:Member() -- Skip the first arg and pass it as a handle
    // Case 2: obj:Static() -- Pass args as is, including the implicit self
    // Case 3: obj:Static(obj) -- Skip the first arg as it duplicates the second arg (for backward compatibility)
    // Case 4: Type.Static() -- Pass args as is
    // Case 5: Type.Member() -- Skip the first arg and pass it as a handle
    // Case 6: GetSingleton("Type"):Static() -- Skip the first arg as it's a dummy
    // Case 7: GetSingleton("Type"):Member() -- Skip the first arg as it's a dummy

    RED4ext::IScriptable* pHandle = nullptr;

    if (aArgs.size() > aArgOffset)
    {
        if (apFunc->flags.isStatic)
        {
            if (aArgs[aArgOffset].is<SingletonReference>())
            {
                ++aArgOffset;
            }
            else if (aArgs[aArgOffset].is<ClassReference>() && aArgs.size() > aArgOffset + 1)
            {
                const auto& cFirst = aArgs[aArgOffset];
                const auto& cSecond = aArgs[aArgOffset + 1];

                if (cFirst.as<sol::object>() == cSecond.as<sol::object>())
                    ++aArgOffset;
            }
        }
        else
        {
            const auto& cArg = aArgs[aArgOffset];

            if (cArg.is<Type>())
            {
                pHandle = reinterpret_cast<RED4ext::IScriptable*>(cArg.as<Type*>()->GetHandle());

                if (cArg.is<StrongReference>() || cArg.is<WeakReference>())
                {
                    ++aArgOffset;

                    if (pHandle && !pHandle->GetType()->IsA(apFunc->GetParent()))
                        pHandle = nullptr;
                }
                else if (cArg.is<SingletonReference>())
                {
                    ++aArgOffset;
                }
            }
        }
    }

    return pHandle;
}

sol::variadic_results RTTIHelper::ExecuteFunction(
    RED4ext::CBaseFunction* apFunc, RED4ext::IScriptable* apContext, sol::variadic_args aLuaArgs, uint64_t aLuaArgOffset, std::string& aErrorMessage, bool aAllowNull) const
{
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 14);
    static thread_local uint32_t s_callDepth = 0u;

    if (!m_pRtti)
        return {};

    const auto* cpEngine = RED4ext::CGameEngine::Get();

    if (!cpEngine || !cpEngine->framework)
        return {};

    // Out params
    // There are cases when out param must be passed to the function.
    // These functions cannot be used until more complex logic for input
    // args is implemented (should check if a compatible arg is actually
    // passed at the expected position + same for optionals).

    if (!apFunc->flags.isStatic && !apContext && !aAllowNull)
    {
        aErrorMessage = fmt::format("Function '{}' context must be '{}'.", apFunc->shortName.ToString(), apFunc->GetParent()->name.ToString());
        return {};
    }

    auto numArgs = aLuaArgs.size() - aLuaArgOffset;
    auto minArgs = 0u;
    auto maxArgs = 0u;

    for (auto i = 0u; i < apFunc->params.size; ++i)
    {
        const auto cpParam = apFunc->params[i];

        if (!cpParam->flags.isOut && cpParam->type != m_pGameInstanceType)
        {
            maxArgs++;

            if (!cpParam->flags.isOptional)
                minArgs++;
        }
    }

    if (numArgs < minArgs || numArgs > maxArgs)
    {
        if (minArgs != maxArgs)
            aErrorMessage = fmt::format("Function '{}' requires from {} to {} parameter(s).", apFunc->shortName.ToString(), minArgs, maxArgs);
        else
            aErrorMessage = fmt::format("Function '{}' requires {} parameter(s).", apFunc->shortName.ToString(), minArgs);

        return {};
    }

    // Allocator management
    ++s_callDepth;

    ScopeGuard allocatorGuard(
        [&]
        {
            --s_callDepth;

            if (s_callDepth == 0u)
                s_scratchMemory.Reset();
        });

    auto pAllocator = TiltedPhoques::Allocator::Get();
    TiltedPhoques::Allocator::Set(&s_scratchMemory);
    TiltedPhoques::Vector<RED4ext::CStackType> callArgs(apFunc->params.size);
    TiltedPhoques::Vector<uint32_t> callArgToParam(apFunc->params.size);
    TiltedPhoques::Vector<bool> argNeedsFree(apFunc->params.size, false);
    TiltedPhoques::Allocator::Set(pAllocator);

    uint32_t callArgOffset = 0u;

    ScopeGuard argsGuard(
        [&]
        {
            for (auto j = 0u; j < callArgOffset; ++j)
            {
                const bool isNew = argNeedsFree[j];

                FreeInstance(callArgs[j], !isNew, isNew, &s_scratchMemory);
            }
        });

    for (auto i = 0u; i < apFunc->params.size; ++i)
    {
        const auto cpParam = apFunc->params[i];

        if (cpParam->type == m_pGameInstanceType)
        {
            callArgs[callArgOffset].type = m_pGameInstanceType;
            callArgs[callArgOffset].value = m_pGameInstance;
        }
        else if (cpParam->flags.isOut)
        {
            // It looks like constructing value with ToRED for out params is not required,
            // and memory allocation is enough.
            callArgs[callArgOffset].type = cpParam->type;
            callArgs[callArgOffset].value = NewPlaceholder(cpParam->type, &s_scratchMemory);
            argNeedsFree[callArgOffset] = true;

            // But ToRED conversion is necessary for implementing required out params.
            // callArgs[callArgOffset] = Scripting::ToRED(sol::nil, cpParam->type, &s_scratchMemory);
        }
        else if (cpParam->flags.isOptional)
        {
            if (aLuaArgOffset < aLuaArgs.size())
            {
                sol::object argValue = aLuaArgs[aLuaArgOffset];
                callArgs[callArgOffset] = Scripting::ToRED(argValue, cpParam->type, &s_scratchMemory);
            }

            // Create a placeholder if value is incompatible or omitted
            if (!callArgs[callArgOffset].value)
            {
                callArgs[callArgOffset].type = cpParam->type;
                callArgs[callArgOffset].value = nullptr;
            }
            else
            {
                ++aLuaArgOffset;
            }
        }
        else if (aLuaArgOffset < aLuaArgs.size())
        {
            sol::object argValue = aLuaArgs[aLuaArgOffset];
            callArgs[callArgOffset] = Scripting::ToRED(argValue, cpParam->type, &s_scratchMemory);
            ++aLuaArgOffset;
        }

        if (!callArgs[callArgOffset].value && !cpParam->flags.isOptional)
        {
            const auto typeName = cpParam->type->GetName();
            aErrorMessage = fmt::format("Function '{}' parameter {} must be {}.", apFunc->shortName.ToString(), i + 1, typeName.ToString());

            return {};
        }

        callArgToParam[callArgOffset] = i;
        ++callArgOffset;
    }

    const bool hasReturnType = apFunc->returnType != nullptr && apFunc->returnType->type != nullptr;

    uint8_t buffer[1000]{0};
    RED4ext::CStackType result;

    if (hasReturnType)
    {
        result.value = &buffer;
        result.type = apFunc->returnType->type;
    }

    const auto success = ExecuteFunction(apFunc, apContext, callArgs, result);

    if (!success)
    {
        aErrorMessage = fmt::format("Function '{}' failed to execute.", apFunc->shortName.ToString());

        return {};
    }

    auto lockedState = m_lua.Lock();

    sol::variadic_results results;

    if (hasReturnType)
    {
        // There is a special case when a non-native function returns a specific class or something that holds that
        // class, which leads to another memory leak. So far the only known class that is causing the issue is
        // gameTargetSearchFilter. When it returned by a non-native function it is wrapped in the Variant or similar
        // structure. To prevent the leak the underlying value must be unwrapped and the wrapper explicitly destroyed.
        // The workaround has been proven to work, but it seems too much for only one known case, so it is not included
        // for now.

        results.emplace_back(Scripting::ToLua(lockedState, result));
        FreeInstance(result, false, false, &s_scratchMemory);
    }

    for (auto i = 0u; i < callArgOffset; ++i)
    {
        const auto cpParam = apFunc->params[callArgToParam[i]];

        if (cpParam->flags.isOut)
            results.emplace_back(Scripting::ToLua(lockedState, callArgs[i]));
    }

    return results;
}

bool RTTIHelper::ExecuteFunction(
    RED4ext::CBaseFunction* apFunc, RED4ext::IScriptable* apContext, TiltedPhoques::Vector<RED4ext::CStackType>& aArgs, RED4ext::CStackType& aResult) const
{
    constexpr auto NopOp = 0;
    constexpr auto ParamOp = 27;
    constexpr auto ParamEndOp = 38;
    constexpr auto MaxCodeSize = 264;
    constexpr auto PointerSize = sizeof(void*);

    char code[MaxCodeSize];
    RED4ext::CStackFrame frame(nullptr, code);

    for (uint32_t i = 0; i < apFunc->params.size; ++i)
    {
        const auto& param = apFunc->params[i];
        const auto& arg = aArgs[i];

        if (param->flags.isOptional && !arg.value)
        {
            *frame.code = NopOp;
            ++frame.code;
        }
        else
        {
            *frame.code = ParamOp;
            ++frame.code;

            *reinterpret_cast<void**>(frame.code) = arg.type;
            frame.code += PointerSize;

            *reinterpret_cast<void**>(frame.code) = arg.value;
            frame.code += PointerSize;
        }
    }

    *frame.code = ParamEndOp;
    frame.code = code; // Rewind

    // This is not intended use, but a trick.
    // The frame we create is the parent of the current call,
    // but we set it to apFunc itself just to make sure it's not null,
    // because there are some functions that expect a non-empty call stack.
    frame.func = apFunc;

    return CallScriptFunction(apFunc, apContext ? apContext : m_pPlayerSystem, &frame, aResult.value, aResult.type);
}

RED4ext::ScriptInstance RTTIHelper::NewPlaceholder(RED4ext::CBaseRTTIType* apType, TiltedPhoques::Allocator* apAllocator) const
{
    auto* pMemory = apAllocator->Allocate(apType->GetSize());
    memset(pMemory, 0, apType->GetSize());
    apType->Construct(pMemory);

    return pMemory;
}

RED4ext::ScriptInstance RTTIHelper::NewInstance(RED4ext::CBaseRTTIType* apType, sol::optional<sol::table> aProps, TiltedPhoques::Allocator* apAllocator) const
{
    if (!m_pRtti)
        return nullptr;

    RED4ext::CClass* pClass = nullptr;

    if (apType->GetType() == RED4ext::ERTTIType::Class)
    {
        pClass = reinterpret_cast<RED4ext::CClass*>(apType);
    }
    else if (apType->GetType() == RED4ext::ERTTIType::Handle)
    {
        auto* pInnerType = reinterpret_cast<RED4ext::CRTTIHandleType*>(apType)->GetInnerType();

        if (pInnerType->GetType() == RED4ext::ERTTIType::Class)
            pClass = reinterpret_cast<RED4ext::CClass*>(pInnerType);
    }

    if (!pClass || pClass->flags.isAbstract)
        return nullptr;

    // AllocInstance() seems to be the only function that initializes an instance.
    // Neither Init() nor InitCls() initializes properties.
    auto* pInstance = pClass->CreateInstance();

    if (aProps.has_value())
        SetProperties(pClass, pInstance, aProps.value());

    return (apType->GetType() == RED4ext::ERTTIType::Handle) ? apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(static_cast<RED4ext::IScriptable*>(pInstance)) : pInstance;
}

sol::object RTTIHelper::NewInstance(RED4ext::CBaseRTTIType* apType, sol::optional<sol::table> aProps) const
{
    if (!m_pRtti)
        return sol::nil;

    TiltedPhoques::StackAllocator<1 << 10> allocator;

    RED4ext::CStackType result;
    result.type = apType;
    result.value = NewInstance(apType, sol::nullopt, &allocator);

    auto lockedState = m_lua.Lock();
    auto instance = Scripting::ToLua(lockedState, result);

    FreeInstance(result, true, true, &allocator);

    if (aProps.has_value())
    {
        const auto pInstance = instance.as<ClassType*>();
        SetProperties(pInstance->GetClass(), pInstance->GetHandle(), aProps);
    }

    return instance;
}

// Create new instance and wrap it in Handle<> if possible
sol::object RTTIHelper::NewHandle(RED4ext::CBaseRTTIType* apType, sol::optional<sol::table> aProps) const
{
    // This method should be preferred over NewInstance() for creating objects in Lua userland.
    // The behavior is similar to what can be seen in scripts, where variables of IScriptable
    // types are declared with the ref<> modifier (which means Handle<>).

    if (!m_pRtti)
        return sol::nil;

    TiltedPhoques::StackAllocator<1 << 10> allocator;

    RED4ext::CStackType result;
    result.type = apType;
    result.value = NewInstance(apType, sol::nullopt, &allocator);

    // Wrap ISerializable descendants in Handle
    if (result.value && apType->GetType() == RED4ext::ERTTIType::Class)
    {
        static auto* s_pHandleType = m_pRtti->GetType(RED4ext::FNV1a64("handle:Activator"));
        static auto* s_pISerializableType = m_pRtti->GetType(RED4ext::FNV1a64("ISerializable"));

        const auto* pClass = reinterpret_cast<RED4ext::CClass*>(apType);

        if (pClass->IsA(s_pISerializableType))
        {
            auto* pInstance = static_cast<RED4ext::ISerializable*>(result.value);
            auto* pHandle = allocator.New<RED4ext::Handle<RED4ext::ISerializable>>(pInstance);

            result.type = s_pHandleType; // To trick converter and deallocator
            result.value = pHandle;
        }
    }

    auto lockedState = m_lua.Lock();
    auto instance = Scripting::ToLua(lockedState, result);

    FreeInstance(result, true, true, &allocator);

    if (aProps.has_value())
    {
        const auto pInstance = instance.as<ClassType*>();
        SetProperties(pInstance->GetClass(), pInstance->GetHandle(), aProps);
    }

    return instance;
}

sol::object RTTIHelper::GetProperty(RED4ext::CClass* apClass, RED4ext::ScriptInstance apHandle, const std::string& acPropName, bool& aSuccess) const
{
    aSuccess = false;

    if (!m_pRtti)
        return sol::nil;

    const auto* pProp = apClass->GetProperty(acPropName.c_str());

    if (!pProp)
        return sol::nil;

    auto lockedState = m_lua.Lock();
    RED4ext::CStackType stackType(pProp->type, pProp->GetValue<uintptr_t*>(apHandle));
    aSuccess = true;

    return Scripting::ToLua(lockedState, stackType);
}

void RTTIHelper::SetProperty(RED4ext::CClass* apClass, RED4ext::ScriptInstance apHandle, const std::string& acPropName, sol::object aPropValue, bool& aSuccess) const
{
    aSuccess = false;

    if (!m_pRtti)
        return;

    const auto* pProp = apClass->GetProperty(acPropName.c_str());

    if (!pProp)
        return;

    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);
    struct ResetAllocator
    {
        ~ResetAllocator() { s_scratchMemory.Reset(); }
    };
    ResetAllocator ___allocatorReset;

    RED4ext::CStackType stackType = Scripting::ToRED(aPropValue, pProp->type, &s_scratchMemory);

    if (stackType.value)
    {
        pProp->SetValue<RED4ext::ScriptInstance>(apHandle, stackType.value);

        FreeInstance(stackType, true, false, &s_scratchMemory);
        aSuccess = true;
    }
}

void RTTIHelper::SetProperties(RED4ext::CClass* apClass, RED4ext::ScriptInstance apHandle, sol::optional<sol::table> aProps) const
{
    bool success;

    for (const auto& cProp : aProps.value())
        SetProperty(apClass, apHandle, cProp.first.as<std::string>(), cProp.second, success);
}

// Check if type is implemented using ClassReference
bool RTTIHelper::IsClassReferenceType(RED4ext::CClass* apClass) const
{
    static constexpr auto s_cHashVector3 = RED4ext::FNV1a64("Vector3");
    static constexpr auto s_cHashVector4 = RED4ext::FNV1a64("Vector4");
    static constexpr auto s_cHashEulerAngles = RED4ext::FNV1a64("EulerAngles");
    static constexpr auto s_cHashQuaternion = RED4ext::FNV1a64("Quaternion");
    static constexpr auto s_cHashItemID = RED4ext::FNV1a64("gameItemID");

    return apClass->name.hash != s_cHashVector3 && apClass->name.hash != s_cHashVector4 && apClass->name.hash != s_cHashEulerAngles && apClass->name.hash != s_cHashQuaternion &&
           apClass->name.hash != s_cHashItemID;
}

void RTTIHelper::FreeInstance(RED4ext::CStackType& aStackType, bool aOwn, bool aNew, TiltedPhoques::Allocator* apAllocator) const
{
    FreeInstance(aStackType.type, aStackType.value, aOwn, aNew, apAllocator);
}

void RTTIHelper::FreeInstance(RED4ext::CBaseRTTIType* apType, void* apValue, bool aOwn, bool aNew, TiltedPhoques::Allocator*) const
{
    if (!apValue)
        return;

    if (aOwn)
    {
        if (apType->GetType() == RED4ext::ERTTIType::Class)
        {
            // Free instances created with AllocInstance()
            if (aNew)
            {
                auto* pClass = reinterpret_cast<RED4ext::CClass*>(apType);

                // Skip basic types
                if (IsClassReferenceType(pClass))
                {
                    pClass->DestructCls(apValue);
                    pClass->GetAllocator()->Free(apValue);
                }
            }
        }
        else
        {
            apType->Destruct(apValue);
        }
    }
    else
    {
        if (aNew || apType->GetType() != RED4ext::ERTTIType::Class)
            apType->Destruct(apValue);
    }

    // Right now it's a workaround that doesn't cover all cases but most.
    // It also requires explicit calls to FreeInstance().
    // Should probably be refactored into a custom allocator that
    // combines ScratchAllocator and managed logic for:
    // 1. Instances created with AllocInstance()
    // 2. DynArray
    // 3. Handle
    // 4. WeakHandle
}
