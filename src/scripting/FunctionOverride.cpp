#include <stdafx.h>

#include "FunctionOverride.h"
#include "Scripting.h"
#include "Utils.h"
#include <reverse/WeakReference.h>
#include <reverse/RTTIHelper.h>
#include <reverse/RTTILocator.h>


static FunctionOverride* s_pOverride = nullptr;

using TRunPureScriptFunction = bool (*)(RED4ext::CBaseFunction* apFunction, RED4ext::CScriptStack*, void*);
using TCallScriptFunction = bool (*)(RED4ext::IFunction* apFunction, RED4ext::IScriptable* apContext,
                                     RED4ext::CStackFrame* apFrame, void* apOut, void* a4);

static TRunPureScriptFunction RealRunPureScriptFunction = nullptr;
static RED4ext::REDfunc<TCallScriptFunction> CallScriptFunction(RED4ext::Addresses::CBaseFunction_InternalExecute);

static constexpr size_t s_cMaxFunctionSize =
    std::max({sizeof(RED4ext::CClassFunction), sizeof(RED4ext::CClassStaticFunction), sizeof(RED4ext::CGlobalFunction)});

inline static size_t GetFunctionSize(RED4ext::CBaseFunction* apFunction)
{
    if (apFunction->flags.isStatic)
    {
        return apFunction->flags.isNative
            ? sizeof(RED4ext::CClassStaticFunction)
            : sizeof(RED4ext::CGlobalFunction);
    }

    return sizeof(RED4ext::CClassFunction);
}

FunctionOverride::FunctionOverride(Scripting* apScripting, Options& aOptions)
    : m_pScripting(apScripting)
{
    s_pOverride = this;

    m_pBuffer = m_pBufferStart = VirtualAlloc(nullptr, m_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    Hook(aOptions);
}

FunctionOverride::~FunctionOverride()
{
    VirtualFree(m_pBufferStart, 0, MEM_RELEASE);

    s_pOverride = nullptr;
}

void* FunctionOverride::MakeExecutable(uint8_t* apData, size_t aSize)
{
    if (std::align(0x10, aSize, m_pBuffer, m_size))
    {
        uint8_t* result = static_cast<uint8_t*>(m_pBuffer);
        m_pBuffer = static_cast<char*>(m_pBuffer) + aSize;
        m_size -= aSize;

        std::memcpy(result, apData, aSize);

        return result;
    }

    return nullptr;
}

void FunctionOverride::Clear()
{
    std::unique_lock lock(s_pOverride->m_lock);

    // Reverse order as we want to swap from most recent to oldest change
    for (auto& [pFunction, pContext] : m_functions)
    {
        auto* pRealFunction = pContext.Trampoline;

        std::array<char, s_cMaxFunctionSize> tmpBuffer;
        size_t funcSize = GetFunctionSize(pRealFunction);

        std::memcpy(&tmpBuffer, pRealFunction, funcSize);
        std::memcpy(pRealFunction, pFunction, funcSize);
        std::memcpy(pFunction, &tmpBuffer, funcSize);
    }

    m_functions.clear();
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
            auto lockedState = chain.pScripting->GetState();
            auto& luaState = lockedState.Get();

            TiltedPhoques::StackAllocator<1 << 13> s_allocator;

            auto pAllocator = TiltedPhoques::Allocator::Get();
            TiltedPhoques::Allocator::Set(&s_allocator);
            TiltedPhoques::Vector<sol::object> args;
            TiltedPhoques::Vector<RED4ext::CStackType> outArgs;
            TiltedPhoques::Allocator::Set(pAllocator);

            auto pContext = apStack->GetContext();
            if (!apFunction->flags.isStatic && pContext)
            {
                const auto weak = RED4ext::WeakHandle<RED4ext::IScriptable>(
                    *(RED4ext::WeakHandle<RED4ext::IScriptable>*)&pContext->ref);
                auto obj = sol::make_object(luaState, WeakReference(lockedState, weak));

                args.push_back(obj);
            }

            for (auto* p : apFunction->params)
            {
                auto* pOffset = p->valueOffset + apStack->args;

                RED4ext::CStackType arg;
                arg.type = p->type;
                arg.value = pOffset;

                args.push_back(Scripting::ToLua(lockedState, arg));

                if (p->flags.isOut)
                    outArgs.push_back(arg);
            }

            RED4ext::CStackType ret;
            ret.value = apStack->GetResultAddr();
            ret.type = apStack->GetType();

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
    uint8_t currentParam = apFrame->currentParam;

    if (!chain.IsEmpty)
    {
        auto lockedState = chain.pScripting->GetState();
        auto& luaState = lockedState.Get();

        // Cheap allocation
        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        auto pAllocator = TiltedPhoques::Allocator::Get();
        TiltedPhoques::Allocator::Set(&s_allocator);
        TiltedPhoques::Vector<sol::object> args;
        TiltedPhoques::Vector<RED4ext::CStackType> outArgs;
        TiltedPhoques::Allocator::Set(pAllocator);

        RED4ext::CStackType self;

        if (!apFunction->flags.isStatic)
        {
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

            const auto ref = (RED4ext::WeakHandle<RED4ext::IScriptable>*)&((RED4ext::IScriptable*)self.value)->ref;
            const auto weak = RED4ext::WeakHandle<RED4ext::IScriptable>(*ref);
            auto obj = sol::make_object(luaState, WeakReference(lockedState, weak));

            args.push_back(obj);
        }

        // Nasty way of popping all args
        for (auto& pArg : apFunction->params)
        {
            auto* pType = pArg->type;
            auto* pAllocator = pType->GetAllocator();

            auto* pInstance = pAllocator->AllocAligned(pType->GetSize(), pType->GetAlignment()).memory;
            pType->Init(pInstance);

            bool isScriptRef = pArg->type->GetType() == RED4ext::ERTTIType::ScriptReference;

            // Exception here we need to allocate the inner object as well
            if (isScriptRef)
            {
                auto* pInnerType = ((RED4ext::CRTTIScriptReferenceType*)pType)->innerType;
                auto* pScriptRef = (RED4ext::ScriptRef<void>*)pInstance;
                pScriptRef->innerType = pInnerType;
                pScriptRef->hash = pInnerType->GetName();
                pScriptRef->ref = pInnerType->GetAllocator()->AllocAligned(pInnerType->GetSize(), pInnerType->GetAlignment()).memory;
                pInnerType->Init(pScriptRef->ref);
            }

            RED4ext::CStackType arg;
            arg.type = pArg->type;
            arg.value = pInstance;

            apFrame->currentParam++;
            apFrame->unk30 = 0;
            apFrame->unk38 = 0;
            const auto opcode = *(apFrame->code++);
            RED4ext::OpcodeHandlers::Run(opcode, (RED4ext::IScriptable*)apFrame->context, apFrame, pInstance, isScriptRef ? pInstance : nullptr);

            args.push_back(Scripting::ToLua(lockedState, arg));

            if (pArg->flags.isOut)
            {
                // This is an original arg, pInstance contains copy
                if (apFrame->unk30)
                    arg.value = reinterpret_cast<RED4ext::ScriptInstance>(apFrame->unk30);

                outArgs.push_back(arg);
            }

            // Release inner values
            if (isScriptRef)
            {
                auto* pScriptRef = (RED4ext::ScriptRef<void>*)pInstance;
                pScriptRef->innerType->Destroy(pScriptRef->ref);
                pScriptRef->innerType->GetAllocator()->Free(pScriptRef->ref);
                pScriptRef->ref = nullptr;
            }

            if (!pArg->flags.isOut || apFrame->unk30)
            {
                pType->Destroy(pInstance);
                pAllocator->Free(pInstance);
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

bool FunctionOverride::ExecuteChain(const CallChain& aChain, std::shared_lock<std::shared_mutex>& aLock,
                                    RED4ext::IScriptable* apContext, TiltedPhoques::Vector<sol::object>* apOrigArgs,
                                    RED4ext::CStackType* apResult, TiltedPhoques::Vector<RED4ext::CStackType>* apOutArgs,
                                    RED4ext::CScriptStack* apStack, RED4ext::CStackFrame* apFrame,
                                    char* apCode, uint8_t aParam)
{
    auto lockedState = aChain.pScripting->GetState();
    auto& luaState = lockedState.Get();

    if (!aChain.Before.empty())
    {
        for (const auto& call : aChain.Before)
        {
            const auto result = call->ScriptFunction(as_args(*apOrigArgs));

            if (!result.valid())
            {
                auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());
            }
        }
    }

    auto* pRealFunction = aChain.Trampoline;
    bool realRetValue = true;

    if (!aChain.Overrides.empty())
    {
        sol::object luaContext = pRealFunction->flags.isStatic ? sol::nil : apOrigArgs->at(0);
        TiltedPhoques::Vector<sol::object> luaArgs(apOrigArgs->begin() + (pRealFunction->flags.isStatic ? 0 : 1),
                                                   apOrigArgs->end());

        auto luaWrapped = WrapNextOverride(aChain, 0, luaState, luaContext, luaArgs, pRealFunction, apContext, aLock);
        auto luaResult = luaWrapped(as_args(luaArgs));

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
                for (auto i = 0; i < apOutArgs->size(); ++i)
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
        for (const auto& call : aChain.After)
        {
            const auto result = call->ScriptFunction(as_args(*apOrigArgs));

            if (!result.valid())
            {
                auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());
            }
        }
    }

    if (aChain.CollectGarbage)
        s_pOverride->m_pScripting->CollectGarbage();

    return realRetValue;
}

sol::function FunctionOverride::WrapNextOverride(const CallChain& aChain, int aStep,
    sol::state& aLuaState, sol::object& aLuaContext, TiltedPhoques::Vector<sol::object>& aLuaArgs,
    RED4ext::CBaseFunction* apRealFunction, RED4ext::IScriptable* apRealContext,
    std::shared_lock<std::shared_mutex>& aLock)
{
    if (aStep == aChain.Overrides.size())
    {
        return MakeSolFunction(aLuaState,
            [&](sol::variadic_args aWrapArgs, sol::this_state aState, sol::this_environment aEnv) -> sol::variadic_results {
                std::string errorMessage;
                sol::variadic_results results = RTTIHelper::Get().ExecuteFunction(apRealFunction, apRealContext, aWrapArgs, 0, errorMessage);

                if (!errorMessage.empty())
                {
                    const sol::environment cEnv = aEnv;
                    auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                    logger->error("Error: {}", errorMessage);

                    aLock.unlock();

                    luaL_error(aState, errorMessage.c_str());
                    return {};
                }

                return results;
            });
    }

    return MakeSolFunction(aLuaState,
        [&, aStep](sol::variadic_args aWrapArgs, sol::this_state aState, sol::this_environment aEnv) -> sol::variadic_results {
            auto call = (aChain.Overrides.rbegin() + aStep)->get();

            for (auto i = 0; i < apRealFunction->params.size; ++i)
            {
                if (i < aWrapArgs.leftover_count())
                    aLuaArgs[i] = aWrapArgs[i];
                else
                    aLuaArgs[i] = (sol::object)sol::nil;
            }

            auto next = WrapNextOverride(aChain, aStep + 1, aLuaState, aLuaContext, aLuaArgs, apRealFunction, apRealContext, aLock);
            auto result = aLuaContext == sol::nil
                ? call->ScriptFunction(as_args(aLuaArgs), next)
                : call->ScriptFunction(aLuaContext, as_args(aLuaArgs), next);

            if (!result.valid())
            {
                const sol::environment cEnv = call->Environment;
                auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());

                aLock.unlock();

                luaL_error(aState, result.get<sol::error>().what());
                return {};
            }

            sol::variadic_results results;
            for (const auto element : result)
                results.push_back(element);

            return results;
        });
}

void FunctionOverride::Hook(Options& aOptions) const
{
    auto& gameImage = aOptions.GameImage;

    {
        const mem::pattern cPattern("40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B");
        const mem::default_scanner cScanner(cPattern);
        RealRunPureScriptFunction = cScanner(gameImage.TextRegion).as<TRunPureScriptFunction>();
        if (!RealRunPureScriptFunction)
            spdlog::error("Could not find pure run script function!");
        else
        {
            auto* pLocation = RealRunPureScriptFunction;
            if (MH_CreateHook(pLocation, &FunctionOverride::HookRunPureScriptFunction,
                              reinterpret_cast<void**>(&RealRunPureScriptFunction)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook RealRunScriptFunction function!");
            else
                spdlog::info("RealRunScriptFunction function hook complete!");
        }
    }

     {
        const mem::pattern cPattern("48 89 5C 24 08 57 48 83 EC 40 8B F9 48 8D 54 24 30 48 8B 0D ?? ?? ?? ?? 41 B8 B8 00 00 00");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            auto* pFirstLocation = pLocation + 0x1A;
            auto* pSecondLocation = pLocation + 0x3A;

            if (*pFirstLocation == sizeof(RED4ext::CScriptedFunction) &&
                *pSecondLocation == sizeof(RED4ext::CScriptedFunction))
            {
                DWORD oldProtect;
                VirtualProtect(pLocation, 0x40, PAGE_READWRITE, &oldProtect);
                *pFirstLocation = *pSecondLocation = std::max(s_cMaxFunctionSize, sizeof(RED4ext::CScriptedFunction));
                VirtualProtect(pLocation, 0x40, oldProtect, &oldProtect);

                spdlog::info("Override function allocator patched!");
            }
            else
                spdlog::error("Could not fix allocator for override functions!");
        }
    }
}

void FunctionOverride::Override(const std::string& acTypeName, const std::string& acFullName,
                                sol::protected_function aFunction, sol::environment aEnvironment,
                                bool aAbsolute, bool aAfter, bool aCollectGarbage)
{
    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pClassType = pRtti->GetClass(acTypeName.c_str());

    if (!pClassType)
    {
        auto* pNativeCName = pRtti->scriptToNative.Get(RED4ext::CName(acTypeName.c_str()));

        if (!pNativeCName)
        {
            spdlog::get("scripting")->error("Class type {} not found", acTypeName);
            return;
        }

        pClassType = pRtti->GetClass(*pNativeCName);
    }

    // Get the real function
    auto* pRealFunction = pClassType->GetFunction(RED4ext::FNV1a(acFullName.c_str()));

    if (!pRealFunction)
    {
        pRealFunction = reinterpret_cast<RED4ext::CClassFunction*>(RTTIHelper::Get().FindFunction(pClassType, RED4ext::FNV1a(acFullName.c_str())));

        if (!pRealFunction)
        {
            spdlog::get("scripting")->error("Function {} in class {} does not exist", acFullName, acTypeName);
            return;
        }
    }

    std::unique_lock lock(m_lock);

    CallChain* pEntry = nullptr;
    auto itor = m_functions.find(pRealFunction);

    // This function was never hooked
    if (itor == std::end(m_functions))
    {
        m_functions[pRealFunction] = {};
        pEntry = &m_functions[pRealFunction];

        RED4ext::CBaseFunction* pFunc;

        if (!m_trampolines.contains(pRealFunction))
        {
            /*
            sub rsp, 56
            mov rax, 0xDEADBEEFC0DEBAAD
            mov qword ptr[rsp + 32], rax
            mov rax, 0xDEADBEEFC0DEBAAD
            call rax
            add rsp, 56
            ret
            */
            uint8_t payload[] = {0x48, 0x83, 0xEC, 0x38, 0x48, 0xB8, 0xAD, 0xBA, 0xDE, 0xC0, 0xEF, 0xBE,
                                 0xAD, 0xDE, 0x48, 0x89, 0x44, 0x24, 0x20, 0x48, 0xB8, 0xAD, 0xBA, 0xDE,
                                 0xC0, 0xEF, 0xBE, 0xAD, 0xDE, 0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x38, 0xC3};

            auto funcAddr = reinterpret_cast<uintptr_t>(&FunctionOverride::HandleOverridenFunction);

            std::memcpy(payload + 6, &pRealFunction, 8);
            std::memcpy(payload + 21, &funcAddr, 8);

            using TNativeScriptFunction = void (*)(RED4ext::IScriptable*, RED4ext::CStackFrame*, void*, int64_t);
            auto* pExecutablePayload = static_cast<TNativeScriptFunction>(MakeExecutable(payload, std::size(payload)));

            if (pRealFunction->flags.isStatic)
            {
                if (pRealFunction->flags.isNative)
                {
                    pFunc = RED4ext::CClassStaticFunction::Create(pClassType, acFullName.c_str(), acFullName.c_str(),
                                                                  pExecutablePayload, pRealFunction->flags);
                    reinterpret_cast<RED4ext::CClassStaticFunction*>(pFunc)->parent = pRealFunction->parent;
                }
                else
                {
                    pFunc = RED4ext::CGlobalFunction::Create(acFullName.c_str(), acFullName.c_str(), pExecutablePayload);
                }
            }
            else
            {
                pFunc = RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acFullName.c_str(),
                                                        pExecutablePayload, pRealFunction->flags);
                reinterpret_cast<RED4ext::CClassFunction*>(pFunc)->parent = pRealFunction->parent;
            }

            pFunc->fullName = pRealFunction->fullName;
            pFunc->shortName = pRealFunction->shortName;

            pFunc->returnType = pRealFunction->returnType;
            for (auto* p : pRealFunction->params)
            {
                pFunc->params.PushBack(p);
            }

            for (auto* p : pRealFunction->localVars)
            {
                pFunc->localVars.PushBack(p);
            }

            pFunc->unk20 = pRealFunction->unk20;
            pFunc->bytecode = pRealFunction->bytecode;
            pFunc->unk48 = pRealFunction->unk48;
            pFunc->unkAC = pRealFunction->unkAC;
            pFunc->flags = pRealFunction->flags;
            pFunc->flags.isNative = true;

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
        std::array<char, s_cMaxFunctionSize> tmpBuffer;
        size_t funcSize = GetFunctionSize(pRealFunction);

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
