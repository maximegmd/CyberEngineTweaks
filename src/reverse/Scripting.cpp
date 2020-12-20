#include "Scripting.h"

#include <vector>
#include <Windows.h>
#include <spdlog/spdlog.h>

#include "Engine.h"
#include "Options.h"
#include "Pattern.h"
#include "RTTI.h"
#include "Utils.h"

struct Unk523
{
    int64_t unk0;
    uint64_t unk8;
};

struct CScriptableStackFrame
{
    int64_t vtbl;
    int64_t unk8;
    int64_t unk10;
    int64_t scriptable18;
    int64_t scriptable20;
    int64_t unk28;
    int64_t args;
    int32_t argCount;
    int64_t unk40;
};

bool Scripting::Execute(const std::string& aCommand, std::string& aReturnMessage)
{
    if (aCommand.empty())
    {
        aReturnMessage = "Empty command.";
        return false;
    }

    const auto paramStart = aCommand.find('(');
    auto funcName = aCommand.substr(0, paramStart);
    trim(funcName);
    if (funcName.empty())
    {
        aReturnMessage = "Empty function.";
        return false;
    }

    std::vector<REDString> redArgs;
    redArgs.reserve(100);
    redArgs.emplace_back(funcName.c_str());

    if (paramStart != std::string::npos)
    {
        size_t tokenPos = paramStart + 1;
        std::string token;

        auto paramEnd = aCommand.find(')', tokenPos);
        if (paramEnd == std::string::npos)
            paramEnd = aCommand.length();

        size_t delimPos;
        while ((delimPos = aCommand.find(',', tokenPos)) != std::string::npos)
        {
            token = aCommand.substr(tokenPos, delimPos - tokenPos);
            trim(token);
            redArgs.emplace_back(token.c_str());
            tokenPos = delimPos + 1;
        }

        token = aCommand.substr(tokenPos, paramEnd - tokenPos);
        trim(token);
        redArgs.emplace_back(token.c_str());
    }

    uintptr_t arg0Rtti = 0;
    uintptr_t argiRtti = 0;
    uintptr_t ctorOffset = 0;
    uintptr_t execOffset = 0;

    if (Options::Get().GameImage.version == Image::MakeVersion(1, 4))
    {
        arg0Rtti = 0x1442FD030 - 0x140000000;
        argiRtti = 0x143C62438 - 0x140000000;
        ctorOffset = 0x140270370 - 0x140000000;
        execOffset = 0x1402254A0 - 0x140000000;
    }
    else if (Options::Get().GameImage.version == Image::MakeVersion(1, 5))
    {
        arg0Rtti = 0x1442BC710 - 0x140000000;
        argiRtti = 0x143C22238 - 0x140000000;
        ctorOffset = 0x14026F8A0 - 0x140000000;
        execOffset = 0x1402249F0 - 0x140000000;
    }

    auto* const type = CRTTISystem::Get()->GetType<CClass>(REDString::Hash("cpPlayerSystem"));
    auto* const engine = CGameEngine::Get();
    auto* unk10 = engine->framework->unk10;

    auto arg0RttiPtr = *(uintptr_t*)(arg0Rtti + (uintptr_t)GetModuleHandle(nullptr));
    auto argiRttiPtr = *(uintptr_t*)(argiRtti + (uintptr_t)GetModuleHandle(nullptr));
    if (!arg0RttiPtr || !argiRttiPtr)
    {
        aReturnMessage = "Game has not completed initialization. Wait until the title screen.";
        return false;
    }

    auto func = CRTTISystem::Get()->GetGlobalFunction(REDString::Hash(funcName.c_str()));
    if (!func)
    {
        aReturnMessage = "Function '" + funcName + "' not found or is not a global.";
        return false;
    }

    if (func->parameter_count < 1 || *func->parameters[0] != arg0RttiPtr)
    {
        aReturnMessage = "Function '" + funcName + "' parameter 0 must be ScriptGameInstance.";
        return false;
    }

    if (func->parameter_count != redArgs.size())
    {
        aReturnMessage = "Function '" + funcName + "' expects " +
            std::to_string(func->parameter_count - 1) + " parameters, not " +
            std::to_string(redArgs.size() - 1) + ".";
        return false;
    }

    for (uint32_t i = 1; i < func->parameter_count; i++)
    {
        if (*func->parameters[i] != argiRttiPtr)
        {
            aReturnMessage = "Function '" + funcName + "' parameter " + std::to_string(i) + " must be String.";
            return false;
        }
    }

    const auto scriptable = unk10->GetTypeInstance(type);

    uint64_t a1 = *(uintptr_t*)(scriptable + 0x40);

    Unk523 args[4];
    args[0].unk0 = arg0RttiPtr;
    args[0].unk8 = (uint64_t)&a1;

    for(auto i = 1u; i < redArgs.size(); ++i)
    {
        args[i].unk0 = argiRttiPtr;
        args[i].unk8 = (uint64_t)&redArgs[i];
    }

    CScriptableStackFrame stack;
    auto script40 = *(uintptr_t*)(scriptable + 0x40);
    auto script40100 = *(uintptr_t*)(script40 + 0x100);

    using ctor_t = CScriptableStackFrame * (*)(CScriptableStackFrame* aThis, __int64 aScriptable, Unk523* aArgs,
        int aArgsCount, __int64 a5, __int64* a6);
    ctor_t ctor = (ctor_t)(ctorOffset + (uintptr_t)GetModuleHandle(nullptr));

    //Result result;

    ctor(&stack, scriptable, args, 3, 0, 0);

    using exec_t = bool (*)(CBaseFunction* aThis, CScriptableStackFrame* stack);
    exec_t exec = (exec_t)(execOffset + (uintptr_t)GetModuleHandle(nullptr));

    return exec(func, &stack);
}

Result::Result()
{
    static auto* ptr = FindSignature({
        0x48, 0x89, 0xB4, 0x24, 0xD8, 0x01, 0x00, 0x00, 0xB9,
        0x05, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xA4, 0x24, 0xA0,
        0x01, 0x00, 0x00, 0x66, 0x3B, 0xC1, 0x4C, 0x89, 0xAC,
        0x24, 0x98, 0x01, 0x00, 0x00, 0x4C, 0x8D, 0x25 }) + 0x24;

    output = reinterpret_cast<REDString*>(ptr + *reinterpret_cast<int32_t*>(ptr) + 4);
}
