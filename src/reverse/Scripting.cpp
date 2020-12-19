#include "Scripting.h"

#include <vector>
#include <Windows.h>
#include <spdlog/spdlog.h>

#include "Engine.h"
#include "Pattern.h"
#include "RTTI.h"
#include "Utils.h"

using TExec = bool(void* apThis, ScriptArgs* apArgs, Result* apResult, uintptr_t apScriptable);
auto* RealExec = (TExec*)(0x25FB960 + reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)));

bool Scripting::Execute(const std::string& aCommand, std::string& aReturnMessage)
{
    const auto argsStart = aCommand.find_first_of('(');
    const auto argsEnd = aCommand.find_first_of(')');

    const auto funcName = aCommand.substr(0, argsStart);

    std::string s = aCommand.substr(argsStart + 1, argsEnd - argsStart - 1);
    const std::string delimiter = ",";

    std::vector<REDString> redArgs;
    redArgs.reserve(100);
    redArgs.emplace_back(funcName.c_str());

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        trim(token);
        redArgs.emplace_back(token.c_str());
        s.erase(0, pos + delimiter.length());
    }
    trim(s);
    redArgs.emplace_back(s.c_str());

    auto* const type = CRTTISystem::Get()->GetType<CClass>(REDString::Hash("cpPlayerSystem"));
    auto* const engine = CGameEngine::Get();
    auto* unk10 = engine->framework->unk10;

    const auto scriptable = unk10->GetTypeInstance(type);

    ScriptArgs args{};
    args.args = redArgs.data();
    args.argCount = redArgs.size() & 0xFFFFFFFF;

    Result result;

    if (!RealExec(nullptr, &args, &result, scriptable))
    {
        aReturnMessage = result.output->ToString();
        return false;
    }

    return true;
}

Result::Result()
{
    static auto* ptr = FindSignature({ 0x4C,0x8D,0x25,0x03,0x63,0xAF,0x02,0x4C
        ,0x89,0xBC,0x24,0x90,0x01,0x00,0x00,0x66
        ,0x0F,0x42,0xC1,0x0F,0x29,0xBC,0x24,0x70
        ,0x01,0x00 }) + 3;

    output = reinterpret_cast<REDString*>(ptr + *reinterpret_cast<int32_t*>(ptr) + 4);
}
