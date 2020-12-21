#include "Scripting.h"
#include "RED4ext/Scripting.hpp"

#include <algorithm>
#include <vector>
#include <spdlog/spdlog.h>

#include "Options.h"
#include "RED4ext/REDreverse/CString.hpp"
#include "overlay/Overlay.h"

#include "GameOptions.h"

Scripting::Scripting()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math);

    m_lua.new_usertype<Scripting>("__Game",
        sol::meta_function::index, &Scripting::Index);

    m_lua.new_usertype<GameOptions>("GameOptions",
        "new", sol::no_constructor,

        "Print", &GameOptions::Print,

        "Get", &GameOptions::Get,
        "GetBool", &GameOptions::GetBool,
        "GetInt", &GameOptions::GetInt,
        "GetFloat", &GameOptions::GetFloat,

        "Set", &GameOptions::Set,
        "SetBool", &GameOptions::SetBool,
        "SetInt", &GameOptions::SetInt,
        "SetFloat", &GameOptions::SetFloat,

        "Toggle", &GameOptions::Toggle,
        "Dump", &GameOptions::Dump);

    m_lua.globals()["Game"] = this;

    m_lua["print"] = [](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::ostringstream oss;
        sol::state_view s(L);
        for(auto& v : args)
        {
            std::string str = s["tostring"](v.get<sol::object>());
            oss << str << " ";
        }
        Overlay::Get().Log(oss.str());
    };
}

Scripting& Scripting::Get()
{
    static Scripting s_instance;
    return s_instance;
}

bool Scripting::ExecuteLua(const std::string& aCommand)
{
    try
    {
        m_lua.script(aCommand);
    }
    catch(std::exception& e)
    {
        Overlay::Get().Log( e.what());
        return false;
    }

    return true;
}

sol::protected_function Scripting::Index(Scripting& aThis, const std::string& acName)
{
    return aThis.InternalIndex(acName);
}

sol::protected_function Scripting::InternalIndex(const std::string& acName)
{
    sol::state_view s(m_lua);
    return make_object(s, [this, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        bool code = this->Execute(name, args, env, L, result);
        if(!code)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return std::make_tuple(code, result);
    });
}

bool Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    std::vector<RED4ext::REDreverse::CString> redArgs;

    for (auto v : aArgs)
    {
        std::string s = m_lua["tostring"](v.get<sol::object>());
        redArgs.emplace_back(s.c_str());
    }

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    auto* pFunc = pRtti->GetGlobalFunction(RED4ext::FNV1a(aFuncName.c_str()));

    if (!pFunc)
    {
        aReturnMessage = "Function '" + aFuncName + "' not found or is not a global.";
        return false;
    }

    if (pFunc->params.size - 1 != redArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' expects " +
            std::to_string(pFunc->params.size - 1) + " parameters, not " +
            std::to_string(redArgs.size()) + ".";
        return false;
    }

    auto engine = RED4ext::REDreverse::CGameEngine::Get();
    auto unk10 = engine->framework->unk10;

    using CStackType = RED4ext::REDreverse::CScriptableStackFrame::CStackType;
    std::vector<CStackType> args(redArgs.size() + 1);
    args[0].type = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[0]);
    args[0].value = &unk10;

    for(auto i = 0; i < redArgs.size(); ++i)
    {
        args[i + 1].type = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[i + 1]);
        args[i + 1].value = &redArgs[i];
    }

    CStackType result;
    /*if (aOut)
    {
        result.value = aOut;
    }*/

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    static const auto cpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");

    const auto* type = pRtti->GetType(cpPlayerSystem);
    const auto* pScriptable = unk10->GetTypeInstance(type);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, pScriptable, args.data(),
                                                          static_cast<uint32_t>(args.size()),  nullptr, 0);
    const auto success = pFunc->Call(stack);

    return success;
}
