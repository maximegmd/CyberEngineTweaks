#include <stdafx.h>

#include "FunctionOverride.h"
#include "Scripting.h"
#include "Utils.h"
#include <reverse/WeakReference.h>
#include <reverse/RTTIHelper.h>
#include <xbyak/xbyak.h>

namespace
{

FunctionOverride* s_pOverride = nullptr;

using TRunPureScriptFunction = bool (*)(RED4ext::CBaseFunction* apFunction, RED4ext::CScriptStack*, void*);
using TCreateFunction = void* (*)(void* apMemoryPool, size_t aSize);
using TCallScriptFunction = bool (*)(RED4ext::IFunction* apFunction, RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, void* apOut, void* a4);

TRunPureScriptFunction RealRunPureScriptFunction = nullptr;
TCreateFunction RealCreateFunction = nullptr;
RED4ext::RelocFunc<TCallScriptFunction> CallScriptFunction(RED4ext::Addresses::CBaseFunction_InternalExecute);

constexpr size_t s_cMaxFunctionSize = std::max({sizeof(RED4ext::CClassFunction), sizeof(RED4ext::CClassStaticFunction), sizeof(RED4ext::CGlobalFunction)});

size_t GetFunctionSize(RED4ext::CBaseFunction* apFunction)
{
    if (apFunction->flags.isStatic)
    {
        return apFunction->flags.isNative ? sizeof(RED4ext::CClassStaticFunction) : sizeof(RED4ext::CGlobalFunction);
    }

    return sizeof(RED4ext::CClassFunction);
}

struct OverrideCodegen : Xbyak::CodeGenerator
{
    OverrideCodegen(uintptr_t apRealFunc, uintptr_t apTrampoline)
    {
        sub(rsp, 56);
        mov(rax, apRealFunc);
        mov(ptr[rsp + 32], rax);
        mov(rax, apTrampoline);
        call(rax);
        add(rsp, 56);
        ret();
    }
};

} // namespace

FunctionOverride::FunctionOverride(Scripting* apScripting)
    : m_pScripting(apScripting)
{
    s_pOverride = this;

    m_pBuffer = m_pBufferStart = VirtualAlloc(nullptr, m_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    Hook();
}

FunctionOverride::~FunctionOverride()
{
    VirtualFree(m_pBufferStart, 0, MEM_RELEASE);

    s_pOverride = nullptr;
}

void* FunctionOverride::MakeExecutable(const uint8_t* apData, size_t aSize)
{
    if (std::align(0x10, aSize, m_pBuffer, m_size))
    {
        auto* result = static_cast<uint8_t*>(m_pBuffer);
        m_pBuffer = static_cast<char*>(m_pBuffer) + aSize;
        m_size -= aSize;

        std::memcpy(result, apData, aSize);

        return result;
    }

    return nullptr;
}

void FunctionOverride::Refresh()
{
    for (auto& [pFunction, pContext] : s_pOverride->m_functions)
    {
        CopyFunctionDescription(pContext.Trampoline, pFunction, pContext.Trampoline->flags.isNative);
    }
}

void FunctionOverride::Clear()
{
    std::lock_guard lock(s_pOverride->m_lock);

    // Reverse order as we want to swap from most recent to oldest change
    for (auto& [pFunction, pContext] : m_functions)
    {
        auto* pRealFunction = pContext.Trampoline;

        size_t funcSize = GetFunctionSize(pRealFunction);

        // TODO - undefined behaviour!
        std::array<char, s_cMaxFunctionSize> tmpBuffer{};
        std::memcpy(&tmpBuffer, pRealFunction, funcSize);
        std::memcpy(pRealFunction, pFunction, funcSize);
        std::memcpy(pFunction, &tmpBuffer, funcSize);
    }

    m_functions.clear();
}

void* FunctionOverride::HookCreateFunction(void* apMemoryPool, size_t)
{
    enum
    {
        kOverrideAllocationSize = std::max(s_cMaxFunctionSize, sizeof(RED4ext::CScriptedFunction))
    };

    return RealCreateFunction(apMemoryPool, kOverrideAllocationSize);
}

bool FunctionOverride::HookRunPureScriptFunction(RED4ext::CClassFunction* apFunction, RED4ext::CScriptStack* apStack, RED4ext::CStackFrame* a3)
{
    if (apFunction->flags.isNative == 1 && s_pOverride)
    {
        std::shared_lock lock(s_pOverride->m_lock);

        const auto itor = s_pOverride->m_functions.find(apFunction);
        if (itor == std::end(s_pOverride->m_functions))
        {
            spdlog::get("scripting")->error("Function appears to be hooked but isn't?");
            return false;
        }

        const auto& chain = itor->second;

        if (!chain.IsEmpty)
        {
            TiltedPhoques::StackAllocator<1 << 13> s_allocator;

            const auto pAllocator = TiltedPhoques::Allocator::Get();
            TiltedPhoques::Allocator::Set(&s_allocator);
            TiltedPhoques::Vector<sol::object> args(0);
            TiltedPhoques::Vector<RED4ext::CStackType> outArgs;
            TiltedPhoques::Allocator::Set(pAllocator);

            const auto pContext = apStack->GetContext();

            {
                auto lockedState = chain.pScripting->GetLockedState();
                const auto& luaState = lockedState.Get();

                if (!apFunction->flags.isStatic && pContext)
                {
                    args.reserve(apFunction->params.size + 1);

                    const auto weak = RED4ext::WeakHandle(*reinterpret_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(&pContext->ref));
                    args.emplace_back(make_object(luaState, WeakReference(lockedState, weak)));
                }
                else
                {
                    args.reserve(apFunction->params.size);
                }

                for (const auto* p : apFunction->params)
                {
                    auto* pOffset = p->valueOffset + apStack->args;

                    RED4ext::CStackType arg;
                    arg.type = p->type;
                    arg.value = pOffset;

                    args.emplace_back(Scripting::ToLua(lockedState, arg));

                    if (p->flags.isOut)
                        outArgs.emplace_back(arg);
                }
            }

            RED4ext::CStackType ret;
            ret.value = apStack->GetResultAddr();
            ret.type = apStack->GetResultType();

            return ExecuteChain(chain, lock, pContext, &args, &ret, &outArgs, apStack, a3, nullptr, 0);
        }

        if (chain.CollectGarbage)
            s_pOverride->m_pScripting->CollectGarbage();

        auto* pRealFunction = chain.Trampoline;

        lock.unlock();

        return RealRunPureScriptFunction(pRealFunction, apStack, a3);
    }

    return RealRunPureScriptFunction(apFunction, apStack, a3);
}

void FunctionOverride::HandleOverridenFunction(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, void* apOut, void* a4, RED4ext::CClassFunction* apFunction)
{
    if (!s_pOverride)
        return;

    std::shared_lock lock(s_pOverride->m_lock);

    const auto itor = s_pOverride->m_functions.find(apFunction);
    if (itor == std::end(s_pOverride->m_functions))
    {
        spdlog::get("scripting")->error("Function appears to be hooked but isn't?");
        return;
    }

    const auto& chain = itor->second;

    // Save state so we can rollback to it after we popped for ourself
    auto* pCode = apFrame->code;
    const uint8_t currentParam = apFrame->currentParam;

    if (!chain.IsEmpty)
    {
        // Cheap allocation
        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        const auto pAllocator = TiltedPhoques::Allocator::Get();
        TiltedPhoques::Allocator::Set(&s_allocator);
        TiltedPhoques::Vector<sol::object> args(0);
        TiltedPhoques::Vector<RED4ext::CStackType> outArgs;
        TiltedPhoques::Allocator::Set(pAllocator);

        {
            auto lockedState = chain.pScripting->GetLockedState();
            const auto& luaState = lockedState.Get();

            if (!apFunction->flags.isStatic)
            {
                RED4ext::CStackType self;

                if (apContext->valueHolder)
                {
                    self.type = apContext->unk30;
                    self.value = apContext;
                }
                else
                {
                    self.type = apFrame->context->unk30;
                    self.value = apFrame->context;
                }

                args.reserve(apFunction->params.size + 1);

                const auto ref = reinterpret_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(&static_cast<RED4ext::IScriptable*>(self.value)->ref);
                const auto weak = RED4ext::WeakHandle(*ref);
                args.emplace_back(make_object(luaState, WeakReference(lockedState, weak)));
            }
            else
            {
                args.reserve(apFunction->params.size);
            }

            // Nasty way of popping all args
            for (const auto& pArg : apFunction->params)
            {
                auto* pType = pArg->type;
                auto* pTypeAllocator = pType->GetAllocator();

                auto* pInstance = pTypeAllocator->AllocAligned(pType->GetSize(), pType->GetAlignment()).memory;
                std::memset(pInstance, 0, pType->GetSize());
                pType->Construct(pInstance);

                const bool isScriptRef = pArg->type->GetType() == RED4ext::ERTTIType::ScriptReference;

                // Exception here we need to allocate the inner object as well
                if (isScriptRef)
                {
                    auto* pInnerType = static_cast<RED4ext::CRTTIScriptReferenceType*>(pType)->innerType;
                    auto* pScriptRef = static_cast<RED4ext::ScriptRef<void>*>(pInstance);
                    pScriptRef->innerType = pInnerType;
                    pScriptRef->hash = pInnerType->GetName();
                    pScriptRef->ref = pInnerType->GetAllocator()->AllocAligned(pInnerType->GetSize(), pInnerType->GetAlignment()).memory;
                    pInnerType->Construct(pScriptRef->ref);
                }

                RED4ext::CStackType arg;
                arg.type = pArg->type;
                arg.value = pInstance;

                apFrame->currentParam++;
                apFrame->data = nullptr;
                apFrame->dataType = nullptr;
                const auto opcode = *apFrame->code++;
                RED4ext::OpcodeHandlers::Run(opcode, apFrame->context, apFrame, pInstance, isScriptRef ? pInstance : nullptr);

                args.emplace_back(Scripting::ToLua(lockedState, arg));

                if (pArg->flags.isOut)
                {
                    // This is an original arg, pInstance contains copy
                    if (apFrame->data)
                        arg.value = apFrame->data;

                    outArgs.emplace_back(arg);
                }

                // Release inner values
                if (isScriptRef)
                {
                    auto* pScriptRef = static_cast<RED4ext::ScriptRef<void>*>(pInstance);
                    pScriptRef->innerType->Destruct(pScriptRef->ref);
                    pScriptRef->innerType->GetAllocator()->Free(pScriptRef->ref);
                    pScriptRef->ref = nullptr;
                }

                if (!pArg->flags.isOut || apFrame->data)
                {
                    pType->Destruct(pInstance);
                    pTypeAllocator->Free(pInstance);
                }
            }
        }

        apFrame->code++; // skip ParamEnd

        RED4ext::CStackType ret;

        if (apFunction->returnType)
        {
            ret.type = apFunction->returnType->type;
            ret.value = apOut;
        }

        ExecuteChain(chain, lock, apContext, &args, &ret, &outArgs, nullptr, apFrame, pCode, currentParam);
        return;
    }

    if (chain.CollectGarbage)
        s_pOverride->m_pScripting->CollectGarbage();

    auto* pRealFunction = chain.Trampoline;

    lock.unlock();

    CallScriptFunction(pRealFunction, apContext, apFrame, apOut, a4);
}

bool FunctionOverride::ExecuteChain(
    const CallChain& aChain, std::shared_lock<std::shared_mutex>& aLock, RED4ext::IScriptable* apContext, TiltedPhoques::Vector<sol::object>* apOrigArgs,
    RED4ext::CStackType* apResult, TiltedPhoques::Vector<RED4ext::CStackType>* apOutArgs, RED4ext::CScriptStack* apStack, RED4ext::CStackFrame* apFrame, char* apCode,
    uint8_t aParam)
{
    if (!aChain.Before.empty())
    {
        auto lockedState = aChain.pScripting->GetLockedState();

        for (const auto& call : aChain.Before)
        {
            const auto result = call->ScriptFunction(as_args(*apOrigArgs));

            if (!result.valid())
            {
                const auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());
            }
        }
    }

    auto* pRealFunction = aChain.Trampoline;
    bool realRetValue = true;

    if (!aChain.Overrides.empty())
    {
        auto lockedState = aChain.pScripting->GetLockedState();
        auto& luaState = lockedState.Get();

        sol::object luaContext = pRealFunction->flags.isStatic ? sol::nil : apOrigArgs->at(0);
        TiltedPhoques::Vector<sol::object> luaArgs(apOrigArgs->begin() + (pRealFunction->flags.isStatic ? 0 : 1), apOrigArgs->end());

        const auto luaWrapped = WrapNextOverride(aChain, 0, luaState, luaContext, luaArgs, pRealFunction, apContext, aLock);
        const auto luaResult = luaWrapped(as_args(luaArgs));

        if (luaResult.valid())
        {
            auto luaRetOffset = 0;

            if (apResult && apResult->value)
            {
                Scripting::ToRED(luaResult.get<sol::object>(), *apResult);
                ++luaRetOffset;
            }

            if (apOutArgs && !apOutArgs->empty())
            {
                for (auto i = 0; i < static_cast<int>(apOutArgs->size()); ++i)
                {
                    auto luaOutArg = luaResult.get<sol::object>(i + luaRetOffset);

                    if (luaOutArg != sol::nil)
                        Scripting::ToRED(luaOutArg, apOutArgs->at(i));
                }
            }
        }
    }
    else
    {
        if (apStack != nullptr)
        {
            realRetValue = RealRunPureScriptFunction(pRealFunction, apStack, apFrame);
        }
        else
        {
            apFrame->code = apCode;
            apFrame->currentParam = aParam;

            realRetValue = CallScriptFunction(pRealFunction, apContext, apFrame, apResult->value, apResult->type);
        }
    }

    if (!aChain.After.empty())
    {
        auto lockedState = aChain.pScripting->GetLockedState();

        for (const auto& call : aChain.After)
        {
            const auto result = call->ScriptFunction(as_args(*apOrigArgs));

            if (!result.valid())
            {
                const auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());
            }
        }
    }

    if (apOrigArgs && !apOrigArgs->empty())
    {
        auto lockedState = aChain.pScripting->GetLockedState();
        apOrigArgs->resize(0);
    }

    if (aChain.CollectGarbage)
        s_pOverride->m_pScripting->CollectGarbage();

    return realRetValue;
}

sol::function FunctionOverride::WrapNextOverride(
    const CallChain& aChain, int aStep, sol::state& aLuaState, sol::object& aLuaContext, TiltedPhoques::Vector<sol::object>& aLuaArgs, RED4ext::CBaseFunction* apRealFunction,
    RED4ext::IScriptable* apRealContext, std::shared_lock<std::shared_mutex>& aLock)
{
    if (aStep == static_cast<int>(aChain.Overrides.size()))
    {
        return MakeSolFunction(
            aLuaState,
            [&](sol::variadic_args aWrapArgs, sol::this_state aState, sol::this_environment aEnv) -> sol::variadic_results
            {
                std::string errorMessage;
                sol::variadic_results results = RTTIHelper::Get().ExecuteFunction(apRealFunction, apRealContext, aWrapArgs, 0, errorMessage);

                if (!errorMessage.empty())
                {
                    const sol::environment cEnv = aEnv;
                    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                    logger->error("Error: {}", errorMessage);

                    aLock.unlock();

                    luaL_error(aState, errorMessage.c_str());
                    return {};
                }

                return results;
            });
    }

    return MakeSolFunction(
        aLuaState,
        [&, aStep](sol::variadic_args aWrapArgs, sol::this_state aState) -> sol::variadic_results
        {
            const auto call = (aChain.Overrides.rbegin() + aStep)->get();

            for (uint32_t i = 0; i < apRealFunction->params.size; ++i)
            {
                if (static_cast<int>(i) < aWrapArgs.leftover_count())
                    aLuaArgs[i] = aWrapArgs[i];
                else
                    aLuaArgs[i] = static_cast<sol::object>(sol::nil);
            }

            auto next = WrapNextOverride(aChain, aStep + 1, aLuaState, aLuaContext, aLuaArgs, apRealFunction, apRealContext, aLock);
            auto result = aLuaContext == sol::nil ? call->ScriptFunction(as_args(aLuaArgs), next) : call->ScriptFunction(aLuaContext, as_args(aLuaArgs), next);

            if (!result.valid())
            {
                const sol::environment cEnv = call->Environment;
                const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());

                aLock.unlock();

                luaL_error(aState, result.get<sol::error>().what());
                return {};
            }

            sol::variadic_results results;
            for (auto&& element : result)
                results.emplace_back(element);

            return results;
        });
}

void FunctionOverride::Hook() const
{
    {
        const RelocPtr<void> func(Game::Addresses::CScript_RunPureScript);
        RealRunPureScriptFunction = reinterpret_cast<TRunPureScriptFunction>(func.GetAddr());
        if (!RealRunPureScriptFunction)
            Log::Error("Could not find pure run script function!");
        else
        {
            auto* pLocation = RealRunPureScriptFunction;
            if (MH_CreateHook(reinterpret_cast<LPVOID>(pLocation), reinterpret_cast<LPVOID>(HookRunPureScriptFunction), reinterpret_cast<void**>(&RealRunPureScriptFunction)) !=
                    MH_OK ||
                MH_EnableHook(reinterpret_cast<LPVOID>(pLocation)) != MH_OK)
                Log::Error("Could not hook RealRunScriptFunction function!");
            else
                Log::Info("RealRunScriptFunction function hook complete!");
        }
    }

    {
        const RelocPtr<void> func(Game::Addresses::CScript_AllocateFunction);
        RealCreateFunction = reinterpret_cast<TCreateFunction>(func.GetAddr());
        if (!RealCreateFunction)
            Log::Error("Could not find create function!");
        else
        {
            auto* pLocation = RealCreateFunction;
            if (MH_CreateHook(reinterpret_cast<LPVOID>(pLocation), reinterpret_cast<LPVOID>(HookCreateFunction), reinterpret_cast<void**>(&RealCreateFunction)) != MH_OK ||
                MH_EnableHook(reinterpret_cast<LPVOID>(pLocation)) != MH_OK)
                Log::Error("Could not hook RealCreateFunction function!");
            else
                Log::Info("RealCreateFunction function hook complete!");
        }
    }
}

void FunctionOverride::Override(
    const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction, sol::environment aEnvironment, bool aAbsolute, bool aAfter,
    bool aCollectGarbage)
{
    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pClassType = pRtti->GetClass(acTypeName.c_str());

    if (!pClassType)
    {
        const auto* pNativeCName = pRtti->scriptToNative.Get(RED4ext::CName(acTypeName.c_str()));

        if (!pNativeCName)
        {
            spdlog::get("scripting")->error("Class type {} not found", acTypeName);
            return;
        }

        pClassType = pRtti->GetClass(*pNativeCName);
    }

    // Get the real function
    auto* pRealFunction = pClassType->GetFunction(acFullName.c_str());

    if (!pRealFunction)
    {
        pRealFunction = reinterpret_cast<RED4ext::CClassFunction*>(RTTIHelper::Get().FindFunction(pClassType, RED4ext::FNV1a64(acFullName.c_str())));

        if (!pRealFunction)
        {
            spdlog::get("scripting")->error("Function {} in class {} does not exist", acFullName, acTypeName);
            return;
        }
    }

    std::lock_guard lock(m_lock);

    CallChain* pEntry = nullptr;
    const auto itor = m_functions.find(pRealFunction);

    // This function was never hooked
    if (itor == std::end(m_functions))
    {
        m_functions[pRealFunction] = {};
        pEntry = &m_functions[pRealFunction];

        RED4ext::CBaseFunction* pFunc;

        if (!m_trampolines.contains(pRealFunction))
        {
            const auto funcAddr = reinterpret_cast<uintptr_t>(&FunctionOverride::HandleOverridenFunction);

            const OverrideCodegen codegen(reinterpret_cast<uintptr_t>(pRealFunction), funcAddr);

            using TNativeScriptFunction = void (*)(RED4ext::IScriptable*, RED4ext::CStackFrame*, void*, int64_t);
            auto* pExecutablePayload = reinterpret_cast<TNativeScriptFunction>(MakeExecutable(codegen.getCode(), codegen.getSize()));

            if (pRealFunction->flags.isStatic)
            {
                if (pRealFunction->flags.isNative)
                {
                    pFunc = RED4ext::CClassStaticFunction::Create(pClassType, acFullName.c_str(), acFullName.c_str(), pExecutablePayload, pRealFunction->flags);
                    reinterpret_cast<RED4ext::CClassStaticFunction*>(pFunc)->parent = pRealFunction->parent;
                }
                else
                {
                    pFunc = RED4ext::CGlobalFunction::Create(acFullName.c_str(), acFullName.c_str(), pExecutablePayload);
                }
            }
            else
            {
                pFunc = RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acFullName.c_str(), pExecutablePayload, pRealFunction->flags);
                reinterpret_cast<RED4ext::CClassFunction*>(pFunc)->parent = pRealFunction->parent;
            }

            CopyFunctionDescription(pFunc, pRealFunction, true);

            m_trampolines[pRealFunction] = pFunc;
        }
        else
        {
            pFunc = m_trampolines[pRealFunction];
        }

        pEntry->Trampoline = pFunc;
        pEntry->pScripting = m_pScripting;
        pEntry->CollectGarbage = aCollectGarbage;
        pEntry->IsEmpty = true;

        // Swap the content of the real function with the one we just created
        const size_t funcSize = GetFunctionSize(pRealFunction);

        // TODO - undefined behaviour!
        std::array<char, s_cMaxFunctionSize> tmpBuffer{};
        std::memcpy(&tmpBuffer, pRealFunction, funcSize);
        std::memcpy(pRealFunction, pFunc, funcSize);
        std::memcpy(pFunc, &tmpBuffer, funcSize);
    }
    else
    {
        pEntry = &itor.value();
    }

    if (aFunction != sol::nil)
    {
        auto pContext = TiltedPhoques::MakeUnique<Context>();
        pContext->ScriptFunction = std::move(aFunction);
        pContext->Environment = aEnvironment;
        pContext->Forward = !aAbsolute;

        if (aAbsolute)
            pEntry->Overrides.emplace_back(std::move(pContext));
        else if (aAfter)
            pEntry->After.emplace_back(std::move(pContext));
        else
            pEntry->Before.emplace_back(std::move(pContext));

        if (pEntry->IsEmpty)
            pEntry->IsEmpty = false;
    }
}

void FunctionOverride::CopyFunctionDescription(RED4ext::CBaseFunction* aFunc, RED4ext::CBaseFunction* aRealFunc, bool aForceNative)
{
    aFunc->fullName = aRealFunc->fullName;
    aFunc->shortName = aRealFunc->shortName;

    aFunc->returnType = aRealFunc->returnType;

    aFunc->params.Clear();
    for (auto* p : aRealFunc->params)
    {
        aFunc->params.PushBack(p);
    }

    aFunc->localVars.Clear();
    for (auto* p : aRealFunc->localVars)
    {
        aFunc->localVars.PushBack(p);
    }

    aFunc->unk20 = aRealFunc->unk20;
    aFunc->bytecode = aRealFunc->bytecode;
    aFunc->unk48 = aRealFunc->unk48;
    aFunc->unkAC = aRealFunc->unkAC;

    aFunc->flags = aRealFunc->flags;
    aFunc->flags.isNative = aForceNative;
}
