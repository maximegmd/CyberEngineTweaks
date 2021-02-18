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

        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        TiltedPhoques::Allocator::Push(s_allocator);
        TiltedPhoques::Vector<sol::object> args;
        TiltedPhoques::Allocator::Pop();

        auto state = itor->second.pScripting->GetState();

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
            const auto result = call->ScriptFunction(as_args(args), call->Environment);
            if (!call->Forward)
            {
                if (result.valid() && ret.value && ret.type)
                    Scripting::ToRED(result.get<sol::object>(), &ret);

                return true;
            }
        }

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

    // Save state so we can rollback to it after we popped for ourself
    auto* pCode = apFrame->code;

    RED4ext::CStackType self;
    self.type = apContext->classType;
    self.value = apContext;

    {
        // Cheap allocation
        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        TiltedPhoques::Allocator::Push(s_allocator);
        TiltedPhoques::Vector<sol::object> args;
        TiltedPhoques::Allocator::Pop();

        auto state = context.pScripting->GetState();

        args.push_back(Scripting::ToLua(state, self)); // Push self

        // Nasty way of popping all args
        for (auto& pArg : apFunction->params)
        {
            auto* pType = pArg->type;
            auto* pAllocator = pType->GetAllocator();

            auto* pInstance = pAllocator->Alloc(pType->GetSize()).memory;
            pType->Init(pInstance);

            RED4ext::CStackType arg;
            arg.type = pArg->type;
            arg.value = pInstance;

            apFrame->unk30 = 0;
            apFrame->unk38 = 0;
            const auto opcode = *(apFrame->code++);
            RED4ext::OpcodeHandlers::Run(opcode, apFrame->context, apFrame, pInstance, nullptr);
            apFrame->code++; // skip ParamEnd

            args.push_back(Scripting::ToLua(state, arg));

            pType->Destroy(pInstance);
            pAllocator->Free(pInstance);
        }

        const auto& calls = context.Calls;
        for (const auto& call : calls)
        {
            const auto result = call->ScriptFunction(as_args(args), call->Environment);
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

                return;
            }
        }
    }

    RED4ext::IFunction* pRealFunction = context.Trampoline;

    lock.unlock();

    if (pRealFunction)
    {
        // Rollback so the real function will manage to pop everything
        apFrame->code = pCode;

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
        const mem::pattern cPattern("40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B 41 7C");
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
        const mem::pattern cPattern(
            "48 89 5C 24 08 57 48 83 EC 40 8B F9 48 8D 54 24 30 48 8B 0D ?? ?? ?? ?? 41 B8 88 00 00 00");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            auto* pFirstLocation = pLocation + 0x1A;
            auto* pSecondLocation = pLocation + 0x3A;

            if (*pFirstLocation == 0x88 && *pSecondLocation == 0x88)
            {
                DWORD oldProtect;
                VirtualProtect(pLocation, 0x40, PAGE_READWRITE, &oldProtect);
                *pFirstLocation = *pSecondLocation = sizeof(RED4ext::CClassFunction);
                VirtualProtect(pLocation, 0x40, oldProtect, &oldProtect);

                spdlog::info("Override function allocator patched!");
            }
            else
                spdlog::error("Could not fix allocator for override functions!");
        }
    }
}

void FunctionOverride::Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
                                bool aAbsolute, sol::protected_function aFunction, sol::this_environment aThisEnv)
{
    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pClassType = pRtti->GetClass(acTypeName.c_str());

    if (!pClassType)
    {
        spdlog::get("scripting")->error("Class type {} not found", acTypeName);
        return;
    }

    auto pContext = TiltedPhoques::MakeUnique<Context>();
    pContext->ScriptFunction = std::move(aFunction);
    pContext->Environment = aThisEnv;
    pContext->Forward = !aAbsolute;

    // Get the real function
    auto* pRealFunction = pClassType->GetFunction(RED4ext::FNV1a(acFullName.c_str()));

    if (pRealFunction)
    {
        std::unique_lock lock(m_lock);

        auto itor = m_functions.find(pRealFunction);

        // This function was never hooked
        if (itor == std::end(m_functions))
        {
            auto& entry = m_functions[pRealFunction] = {};

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

            auto* const pFunc = RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acShortName.c_str(), pExecutablePayload);

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
            std::copy_n(pRealFunction->unk48, std::size(pRealFunction->unk48), pFunc->unk48);
            pFunc->unk7C = pRealFunction->unk7C;
            pFunc->flags = pRealFunction->flags;
            pFunc->parent = pRealFunction->parent;
            pFunc->flags.isNative = true;

            entry.Trampoline = pFunc;
            entry.pScripting = m_pScripting;

            // Swap the content of the real function with the one we just created
            std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

            std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
            std::memcpy(pRealFunction, pFunc, sizeof(RED4ext::CClassFunction));
            std::memcpy(pFunc, &tmpBuffer, sizeof(RED4ext::CClassFunction));

            entry.Calls.emplace_back(std::move(pContext));
        }
        else
            itor.value().Calls.emplace_back(std::move(pContext));
        
    }
    else
    {
        spdlog::get("scripting")->error("Function {} in class {} does not exist", acFullName, acTypeName);
    }
}
