#include "System.h"

#include "Scripting.h"
#include "RED4ext/Scripting.hpp"

#include <vector>
#include <spdlog/spdlog.h>

#include "Options.h"
#include "RED4ext/REDreverse/CString.hpp"
#include "overlay/Overlay.h"

System::System(sol::state_view aView, std::string aName)
    :m_lua(aView)
    ,m_name(std::move(aName))
{
}

sol::object System::Index(const std::string& acName)
{
    if(const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return InternalIndex(acName);
}

sol::object System::NewIndex(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = aParam;
    return property;
}

sol::protected_function System::InternalIndex(const std::string& acName)
{
    const sol::state_view state(m_lua);
    auto obj = make_object(state, [this, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        auto funcRet = this->Execute(name, args, env, L, result);
        if(!funcRet)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return funcRet;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object System::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    static RED4ext::REDfunc<char* (*)(uint64_t& aHash)> CNamePool_Get({ 0x48, 0x83, 0xEC, 0x38, 0x48,0x8B,0x11,0x48,0x8D,0x4C,0x24,0x20,0xE8 }, 1);

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();

    auto* type = pRtti->GetType<RED4ext::REDreverse::CClass*>(RED4ext::FNV1a(m_name));
    if (!type)
    {
        return make_object(m_lua, nullptr);
    }

    auto* pFunc = type->GetFunction(RED4ext::FNV1a(aFuncName));

    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));
    static auto* pInt32Type = pRtti->GetType(RED4ext::FNV1a("Int32"));

    if (!pFunc)
    {
        aReturnMessage = "Function '" + aFuncName + "' not found in system '" + m_name + "'.";
        return make_object(m_lua, nullptr);
    }

    if (pFunc->params.size != aArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' expects " +
            std::to_string(pFunc->params.size) + " parameters, not " +
            std::to_string(aArgs.size()) + ".";

        return make_object(m_lua, nullptr);
    }

    auto* engine = RED4ext::REDreverse::CGameEngine::Get();
    auto* unk10 = engine->framework->unk10;

    using CStackType = RED4ext::REDreverse::CScriptableStackFrame::CStackType;
    std::vector<CStackType> args(aArgs.size());

    const bool hasReturnType = (pFunc->returnType) != nullptr && (*pFunc->returnType) != nullptr;

    for(auto i = 0; i < aArgs.size(); ++i)
    {
        auto arg = aArgs[i];

        auto* pType = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[i]);
        args[i].type = pType;

        if(pType == pStringType)
        {
            const std::string sstr = m_lua["tostring"](arg.get<sol::object>());
            auto* pMemory = _malloca(sizeof(RED4ext::REDreverse::CString));

            auto* pString = new (pMemory) RED4ext::REDreverse::CString{ sstr.c_str() };
            args[i].value = pString;
        }
        else if(pType == pInt32Type)
        {
            auto* pMemory = static_cast<int32_t*>(_malloca(sizeof(int32_t)));
            *pMemory = arg.get<int32_t>();
            args[i].value = pMemory;
        }
        else
        {
            uint64_t hash = 0;
            pType->GetName(&hash);
            if (hash)
            {
                std::string typeName = CNamePool_Get(hash);
                aReturnMessage = "Function '" + aFuncName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return make_object(m_lua, nullptr);
        }        
    }

    uint64_t value = 0;

    CStackType result;
    if (hasReturnType)
    {
        /*uint64_t hash = 0;
        (*pFunc->returnType)->GetName(&hash);
        if (hash)
        {
            const std::string typeName = CNamePool_Get(hash);
            Overlay::Get().Log("Return type: " + typeName + " type : " + std::to_string((uint32_t)(*pFunc->returnType)->GetType()));
        }*/

        result.value = &value;
    }

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    const auto* pScriptable = unk10->GetTypeInstance(type);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, pScriptable, args.data(),
                                                          static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);
    const auto success = pFunc->Call(stack);
    if (!success)
        return make_object(m_lua, nullptr);

    if(hasReturnType)
    {
        return make_object(m_lua, value);
    }

    return make_object(m_lua, true);
}
