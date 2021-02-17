#include <stdafx.h>

#include "FunctionOverride.h"
#include "LuaVM.h"
#include "Scripting.h"

using TRunPureScriptFunction = bool (*)(RED4ext::CBaseFunction* apFunction, RED4ext::CScriptStack*, void*);
static TRunPureScriptFunction RealRunPureScriptFunction = nullptr;

bool HookRunPureScriptFunction(RED4ext::CBaseFunction* apFunction, RED4ext::CScriptStack* apContext, void* a3)
{
    if (apFunction->flags.isNative == 1)
    {
        TiltedPhoques::StackAllocator<1 << 13> s_allocator;

        TiltedPhoques::Allocator::Push(s_allocator);
        TiltedPhoques::Vector<RED4ext::CStackType> args;
        TiltedPhoques::Allocator::Pop();

        for (auto* p : apFunction->params)
        {
            auto* pOffset = p->valueOffset + apContext->args;

            RED4ext::CStackType arg;
            arg.type = p->type;
            arg.value = pOffset;

            args.push_back(arg);
        }

        RED4ext::CStackType ret;
        ret.value = apContext->GetResultAddr();
        ret.type = apContext->GetType();

        RED4ext::CStack stack(apContext->context20, args.data(), args.size(), &ret);
        return apFunction->Execute(&stack);
    }

    return RealRunPureScriptFunction(apFunction, apContext, a3);
}

FunctionOverride::FunctionOverride(Scripting* apScripting, Options& aOptions)
    : m_pScripting(apScripting)
{
    m_pBuffer = m_pBufferStart = VirtualAlloc(nullptr, m_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    Hook(aOptions);
}

FunctionOverride::~FunctionOverride()
{
    VirtualFree(m_pBufferStart, 0, MEM_RELEASE);
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
    // Reverse order as we want to swap from most recent to oldest change
    for (auto itor = std::rbegin(m_overrides); itor != std::rend(m_overrides); ++itor)
    {
        auto& funcs = *itor;

        auto* pFunc = funcs.NewFunction;

        // Just an added function, not an override
        if (funcs.OldFunction == nullptr)
        {
            auto* pClassType = pFunc->parent;
            auto* pArray = &pClassType->funcs;

            if (pFunc->flags.isStatic)
                pClassType->staticFuncs;

            for (auto*& pItor : *pArray)
            {
                if (pItor == pFunc)
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
            auto* pRealFunction = funcs.OldFunction;

            std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

            std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
            std::memcpy(pRealFunction, pFunc, sizeof(RED4ext::CClassFunction));
            std::memcpy(pFunc, &tmpBuffer, sizeof(RED4ext::CClassFunction));
        }

        /**
         * Don't free, it's likely garbage collected later, freeing causes a crash on load
         *
         * auto* pAllocator = pFunc->GetAllocator();
         * RED4ext::IFunction* pVFunc = pFunc;
         * pAllocator->Free(pFunc);
         */
    }

    m_overrides.clear();

    m_pBuffer = m_pBufferStart;
    m_size = kExecutableSize;
}

void FunctionOverride::HandleOverridenFunction(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, int32_t* apOut, int64_t a4, Context* apCookie)
{
    auto* pRealFunction = apCookie->RealFunction;

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

        auto state = apCookie->pScripting->GetState();

        args.push_back(Scripting::ToLua(state, self)); // Push self

        // Nasty way of popping all args
        for (auto& pArg : apCookie->FunctionDefinition->params)
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
            GetScriptCallArray()[opcode](apFrame->context, apFrame, pInstance, nullptr);
            apFrame->code++; // skip ParamEnd

            args.push_back(Scripting::ToLua(state, arg));

            pType->Destroy(pInstance);
            pAllocator->Free(pInstance);
        }

        const auto result = apCookie->ScriptFunction(as_args(args), apCookie->Environment);

        if (!apCookie->Forward)
        {
            RED4ext::CStackType redResult;
            redResult.type = apCookie->FunctionDefinition->returnType->type;
            redResult.value = apOut;

            if (apOut)
                Scripting::ToRED(result.get<sol::object>(), &redResult);

            return;
        }
    }

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
            if (MH_CreateHook(pLocation, &HookRunPureScriptFunction,
                              reinterpret_cast<void**>(&RealRunPureScriptFunction)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook RealRunScriptFunction function!");
            else
                spdlog::info("RealRunScriptFunction function hook complete!");
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

    auto pContext = TiltedPhoques::MakeUnique<Context>();
    pContext->ScriptFunction = aFunction;
    pContext->Environment = aThisEnv;
    pContext->pScripting = m_pScripting;

    auto pPtr = pContext.get();

    std::memcpy(payload + 6, &pPtr, 8);
    std::memcpy(payload + 21, &funcAddr, 8);

    using TNativeScriptFunction = void (*)(RED4ext::IScriptable*, RED4ext::CStackFrame*, void*, int64_t);

    auto* pExecutablePayload = static_cast<TNativeScriptFunction>(MakeExecutable(payload, std::size(payload)));

    if (!pExecutablePayload)
    {
        spdlog::get("scripting")->error("Unable to create the override payload!");
        return;
    }

    // Get the real function
    auto* const pRealFunction = pClassType->GetFunction(RED4ext::FNV1a(acFullName.c_str()));
    auto* const pFunc =
        RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acShortName.c_str(), pExecutablePayload);

    if (pRealFunction)
    {
        pFunc->returnType = pRealFunction->returnType;
        for (auto* p : pRealFunction->params)
        {
            pFunc->params.PushBack(p);
        }

        pFunc->unk7C = pRealFunction->unk7C;
    }
    pFunc->flags.isNative = true;
    pContext->Forward = !aAbsolute;

    if (!pRealFunction)
    {
        if (pFunc->flags.isStatic)
            pClassType->staticFuncs.PushBack(pFunc);
        else
            pClassType->funcs.PushBack(pFunc);

        pContext->FunctionDefinition = pFunc;
    }
    else
    {
        // Swap the content of the real function with the one we just created
        std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

        std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
        std::memcpy(pRealFunction, pFunc, sizeof(RED4ext::CClassFunction));
        std::memcpy(pFunc, &tmpBuffer, sizeof(RED4ext::CClassFunction));

        // Now pFunc is the real function
        pContext->RealFunction = pFunc;
        pContext->FunctionDefinition = pRealFunction;
    }

    m_overrides.emplace_back(pRealFunction, pFunc, std::move(pContext));
}
