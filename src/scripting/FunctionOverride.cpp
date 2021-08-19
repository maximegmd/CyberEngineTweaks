#include <stdafx.h>

#include "FunctionOverride.h"
#include "Scripting.h"


static FunctionOverride* s_pOverride = nullptr;

using TRunPureScriptFunction = bool (*)(RED4ext::CClassFunction* apFunction, RED4ext::CScriptStack*, void*);
static TRunPureScriptFunction RealRunPureScriptFunction = nullptr;

bool FunctionOverride::HookRunPureScriptFunction(RED4ext::CClassFunction* apFunction, RED4ext::CScriptStack* apContext, void* a3)
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

        bool isOverride = false;

        if (!itor->second.Calls.empty())
        {
            TiltedPhoques::StackAllocator<1 << 13> s_allocator;

            auto pAllocator = TiltedPhoques::Allocator::Get();
            TiltedPhoques::Allocator::Set(&s_allocator);
            TiltedPhoques::Vector<sol::object> args;
            TiltedPhoques::Allocator::Set(pAllocator);

            auto state = itor->second.pScripting->GetState();

            if (apContext->context18)
            {
                RED4ext::CStackType self;
                self.type = reinterpret_cast<RED4ext::IScriptable*>(apContext->context18)->classType;
                self.value = apContext->context18;

                args.push_back(Scripting::ToLua(state, self));
            }

            for (auto* p : apFunction->params)
            {
                auto* pOffset = p->valueOffset + apContext->args;

                RED4ext::CStackType arg;
                arg.type = p->type;
                arg.value = pOffset;

                args.push_back(Scripting::ToLua(state, arg));
            }

            RED4ext::CStackType ret;
            ret.value = apContext->GetResultAddr();
            ret.type = apContext->GetType();

            const auto& calls = itor->second.Calls;
            for (const auto& call : calls)
            {
                const auto result = call->ScriptFunction(as_args(args));

                if (!result.valid())
                {
                    auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                    logger->error(result.get<sol::error>().what());
                }

                if (!call->Forward)
                {
                    if (result.valid() && ret.value && ret.type)
                        Scripting::ToRED(result.get<sol::object>(), &ret);

                    isOverride = true;
                    break;
                }
            }
        }

        if (itor->second.CollectGarbage)
            s_pOverride->m_pScripting->CollectGarbage();

        if (isOverride)
            return true;

        auto* pTrampoline = itor->second.Trampoline;

        lock.unlock();

        return RealRunPureScriptFunction(pTrampoline, apContext, a3);
    }

    return RealRunPureScriptFunction(apFunction, apContext, a3);
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
        // Just an added function, not an override
        if (pContext.Trampoline == nullptr)
        {
            auto* pClassType = pFunction->parent;
            auto* pArray = &pClassType->funcs;

            if (pFunction->flags.isStatic)
                pClassType->staticFuncs;

            for (auto*& pItor : *pArray)
            {
                if (pItor == pFunction)
                {
                    // Swap our self with the last element
                    pItor = *(pArray->End() - 1);
                    // Pop last
                    pArray->size -= 1;

                    break;
                }
            }
        }
        else
        {
            auto* pRealFunction = pContext.Trampoline;

            std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

            std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
            std::memcpy(pRealFunction, pFunction, sizeof(RED4ext::CClassFunction));
            std::memcpy(pFunction, &tmpBuffer, sizeof(RED4ext::CClassFunction));
        }
    }

    m_functions.clear();

    m_pBuffer = m_pBufferStart;
    m_size = kExecutableSize;
}

void FunctionOverride::HandleOverridenFunction(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, int32_t* apOut, int64_t a4, RED4ext::CClassFunction* apFunction)
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

    const auto& context = itor->second;
    bool isOverride = false;

    // Save state so we can rollback to it after we popped for ourself
    auto* pCode = apFrame->code;
    uint8_t currentParam = apFrame->currentParam;

    if (!context.Calls.empty())
    {
        RED4ext::CStackType self;
        if (apContext->valueHolder)
        {
            self.type = apContext->classType;
            self.value = apContext;
        }
        else
        {
            self.type = ((RED4ext::IScriptable*)apFrame->context)->classType;
            self.value = apFrame->context;
        }

        // Cheap allocation
        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        auto pAllocator = TiltedPhoques::Allocator::Get();
        TiltedPhoques::Allocator::Set(&s_allocator);
        TiltedPhoques::Vector<sol::object> args;
        TiltedPhoques::Allocator::Set(pAllocator);

        auto state = context.pScripting->GetState();

        args.push_back(Scripting::ToLua(state, self)); // Push self

        // Nasty way of popping all args
        for (auto& pArg : apFunction->params)
        {
            auto* pType = pArg->type;
            auto* pAllocator = pType->GetAllocator();

            auto* pInstance = pAllocator->Alloc(pType->GetSize()).memory;
            pType->Init(pInstance);

            // Exception here we need to allocate the inner object as well
            if (pArg->type->GetType() == RED4ext::ERTTIType::ScriptReference)
            {
                RED4ext::ScriptRef<void>* pScriptRef = (RED4ext::ScriptRef<void>*)pInstance;
                auto pInnerType = pScriptRef->innerType;

                auto pInnerInstance = pInnerType->GetAllocator()->Alloc(pInnerType->GetSize()).memory;
                pInnerType->Init(pInnerInstance);

                pScriptRef->ref = pInnerInstance;
            }

            RED4ext::CStackType arg;
            arg.type = pArg->type;
            arg.value = pInstance;

            apFrame->currentParam++;
            apFrame->unk30 = 0;
            apFrame->unk38 = 0;
            const auto opcode = *(apFrame->code++);
            RED4ext::OpcodeHandlers::Run(opcode, (RED4ext::IScriptable*)apFrame->context, apFrame, pInstance, nullptr);

            args.push_back(Scripting::ToLua(state, arg));

            // Release inner values
            if (pArg->type->GetType() == RED4ext::ERTTIType::ScriptReference)
            {
                RED4ext::ScriptRef<void>* pScriptRef = (RED4ext::ScriptRef<void>*)pInstance;
                pScriptRef->innerType->Destroy(pScriptRef->ref);
                pScriptRef->innerType->GetAllocator()->Free(pScriptRef->ref);
            }

            pType->Destroy(pInstance);
            pAllocator->Free(pInstance);
        }

        const auto& calls = context.Calls;
        for (const auto& call : calls)
        {
            const auto result = call->ScriptFunction(as_args(args));

            if (!result.valid())
            {
                auto logger = call->Environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
                logger->error(result.get<sol::error>().what());
            }

            if (!call->Forward)
            {
                if (apFunction->returnType)
                {
                    RED4ext::CStackType redResult;
                    redResult.type = apFunction->returnType->type;
                    redResult.value = apOut;

                    if (result.valid() && apOut)
                        Scripting::ToRED(result.get<sol::object>(), &redResult);
                }

                isOverride = true;
                break;
            }
        }
    }

    if (context.CollectGarbage)
        s_pOverride->m_pScripting->CollectGarbage();

    if (isOverride)
        return;

    RED4ext::IFunction* pRealFunction = context.Trampoline;

    lock.unlock();

    if (pRealFunction)
    {
        // Rollback so the real function will manage to pop everything
        apFrame->code = pCode;
        apFrame->currentParam = currentParam;

        using TCallScriptFunction = bool (*)(RED4ext::IFunction * apFunction, RED4ext::IScriptable * apContext,
                                             RED4ext::CStackFrame * apFrame, int32_t * apOut, int64_t a4);
        static RED4ext::REDfunc<TCallScriptFunction> CallScriptFunction(RED4ext::Addresses::CBaseFunction_InternalExecute);

        CallScriptFunction(pRealFunction, apContext, apFrame, apOut, a4);
    }
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
                *pFirstLocation = *pSecondLocation = std::max(sizeof(RED4ext::CClassFunction), sizeof(RED4ext::CScriptedFunction));
                VirtualProtect(pLocation, 0x40, oldProtect, &oldProtect);

                spdlog::info("Override function allocator patched!");
            }
            else
                spdlog::error("Could not fix allocator for override functions!");
        }
    }
}

void FunctionOverride::Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
                                bool aAbsolute, sol::protected_function aFunction, sol::environment aEnvironment,
                                bool aCollectGarbage)
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

    if (pRealFunction)
    {
        std::unique_lock lock(m_lock);

        CallChain* pEntry = nullptr;
        auto itor = m_functions.find(pRealFunction);

        // This function was never hooked
        if (itor == std::end(m_functions))
        {
            m_functions[pRealFunction] = {};
            pEntry = &m_functions[pRealFunction];

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

            auto* const pFunc = RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acShortName.c_str(),
                                                                pExecutablePayload);

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
            std::copy_n(pRealFunction->unk78, std::size(pRealFunction->unk78), pFunc->unk78);
            pFunc->unk48 = pRealFunction->unk48;
            pFunc->unkAC = pRealFunction->unkAC;
            pFunc->flags = pRealFunction->flags;
            pFunc->parent = pRealFunction->parent;
            pFunc->flags.isNative = true;

            pEntry->Trampoline = pFunc;
            pEntry->pScripting = m_pScripting;
            pEntry->CollectGarbage = aCollectGarbage;

            // Swap the content of the real function with the one we just created
            std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

            std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
            std::memcpy(pRealFunction, pFunc, sizeof(RED4ext::CClassFunction));
            std::memcpy(pFunc, &tmpBuffer, sizeof(RED4ext::CClassFunction));
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

            pEntry->Calls.emplace_back(std::move(pContext));
        }
    }
    else
    {
        spdlog::get("scripting")->error("Function {} in class {} does not exist", acFullName, acTypeName);
    }
}
