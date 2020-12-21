#include "Scripting.h"
#include "RED4ext/Scripting.hpp"

#include <vector>
#include <spdlog/spdlog.h>

#include "Options.h"
#include "System.h"
#include "RED4ext/REDreverse/CString.hpp"
#include "overlay/Overlay.h"

Scripting::Scripting()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);

    m_lua.new_usertype<Scripting>("__Game",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Scripting::Index,
        sol::meta_function::new_index, &Scripting::NewIndex);

    m_lua.new_usertype<System>("__System",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &System::Index,
        sol::meta_function::new_index, &System::NewIndex);

    m_lua["Game"] = this;
    m_lua["GetSystem"] = [this](const std::string& acName)
    {
        return this->GetSystem(acName);
    };

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

    m_lua.do_file("plugins/cyber_engine_tweaks/scripts/autoexec.lua");
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

sol::object Scripting::Index(const std::string& acName)
{
    if(const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return InternalIndex(acName);
}

sol::object Scripting::NewIndex(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = aParam;
    return property;
}

sol::object Scripting::GetSystem(const std::string& acName)
{
    auto itor = m_systems.find(acName);
    if (itor != std::end(m_systems))
        return make_object(m_lua, itor->second);

    auto result = m_systems.emplace(std::make_pair(acName, System{ m_lua, acName }));

    return make_object(m_lua, result.first->second);
}

sol::protected_function Scripting::InternalIndex(const std::string& acName)
{
    const sol::state_view state(m_lua);
    auto obj = make_object(state, [this, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        bool code = this->Execute(name, args, env, L, result);
        if(!code)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return std::make_tuple(code, result);
    });

    return NewIndex(acName, std::move(obj));
}

bool Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    static RED4ext::REDfunc<char* (*)(uint64_t& aHash)> CNamePool_Get({ 0x48, 0x83, 0xEC, 0x38, 0x48,0x8B,0x11,0x48,0x8D,0x4C,0x24,0x20,0xE8 }, 1);

    std::vector<RED4ext::REDreverse::CString> redArgs;

    for (auto v : aArgs)
    {
        std::string s = m_lua["tostring"](v.get<sol::object>());
        redArgs.emplace_back(s.c_str());
    }

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    auto* pFunc = pRtti->GetGlobalFunction(RED4ext::FNV1a(aFuncName.c_str()));
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

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

    const bool hasReturnType = (pFunc->returnType) != nullptr && (*pFunc->returnType) != nullptr;

    for(auto i = 0; i < redArgs.size(); ++i)
    {
        auto pType = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[i + 1]);;
        if (pType != pStringType)
        {
            aReturnMessage = "Function '" + aFuncName + "' parameter " + std::to_string(i) + " must be String.";
            return false;
        }
        
        args[i + 1].type = pType;
        args[i + 1].value = &redArgs[i];
    }

    CStackType result;
    if (hasReturnType)
    {
        uint64_t hash = 0;
        (*pFunc->returnType)->GetName(&hash);
        if (hash)
        {
            std::string typeName = CNamePool_Get(hash);
            Overlay::Get().Log("Return type: " + typeName);
        }
    }
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
