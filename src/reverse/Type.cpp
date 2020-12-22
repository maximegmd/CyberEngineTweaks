#include "Type.h"

#include "Scripting.h"
#include "RED4ext/Scripting.hpp"

#include <vector>
#include <spdlog/spdlog.h>


#include "BasicTypes.h"
#include "Options.h"
#include "RED4ext/REDreverse/CString.hpp"
#include "RED4ext/REDreverse/Function.hpp"
#include "overlay/Overlay.h"
#include "RED4ext/REDreverse/CName.hpp"

Type::Type(sol::state_view aView, RED4ext::REDreverse::CClass* apClass, std::string aName, RED4ext::REDreverse::Scripting::IScriptable* apHandle)
    : m_lua(aView)
    , m_pType(apClass)
    , m_name(std::move(aName))
    , m_pHandle(apHandle)
{
}

sol::object Type::Index(const std::string& acName)
{
    if(const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return InternalIndex(acName);
}

sol::object Type::NewIndex(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = std::move(aParam);
    return property;
}

sol::protected_function Type::InternalIndex(const std::string& acName)
{
    const sol::state_view state(m_lua);

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();

    auto* type = pRtti->GetType<RED4ext::REDreverse::CClass*>(RED4ext::FNV1a(m_name));
    if (!type)
    {
        return make_object(m_lua, nullptr);
    }

    auto* pFunc = type->GetFunction(RED4ext::FNV1a(acName));
    if(!pFunc)
    {
        Overlay::Get().Log("Function '" + acName + "' not found in system '" + m_name + "'.");
        return make_object(m_lua, nullptr);
    }

    auto obj = make_object(state, [this, pFunc, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        auto funcRet = this->Execute(pFunc, name, args, env, L, result);
        if(!funcRet)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return funcRet;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object Type::Execute(RED4ext::REDreverse::CClassFunction* apFunc, const std::string& acName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();

    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));
    static auto* pCNameType = pRtti->GetType(RED4ext::FNV1a("CName"));
    static auto* pInt32Type = pRtti->GetType(RED4ext::FNV1a("Int32"));
    static auto* pQuaternionType = pRtti->GetType(RED4ext::FNV1a("Quaternion"));

    if (apFunc->params.size != aArgs.size())
    {
        aReturnMessage = "Function '" + acName + "' expects " +
            std::to_string(apFunc->params.size) + " parameters, not " +
            std::to_string(aArgs.size()) + ".";

        return make_object(m_lua, nullptr);
    }

    auto* engine = RED4ext::REDreverse::CGameEngine::Get();
    auto* unk10 = engine->framework->unk10;

    using CStackType = RED4ext::REDreverse::CScriptableStackFrame::CStackType;
    std::vector<CStackType> args(aArgs.size());

    const bool hasReturnType = (apFunc->returnType) != nullptr && (*apFunc->returnType) != nullptr;

    for(auto i = 0ull; i < aArgs.size(); ++i)
    {
        auto arg = aArgs[i];

        auto* pType = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(apFunc->params.unk0[i]);
        args[i].type = pType;

        void* pMemory = nullptr;

        if(pType == pStringType)
        {
            const std::string sstr = m_lua["tostring"](arg.get<sol::object>());
            pMemory = _malloca(sizeof(RED4ext::REDreverse::CString));

            new (pMemory) RED4ext::REDreverse::CString{ sstr.c_str() };
        }
        else if(pType == pInt32Type)
        {
            pMemory = _malloca(sizeof(int32_t));
            *static_cast<int32_t*>(pMemory) = arg.get<int32_t>();
        }
        else if(pType == pQuaternionType)
        {
            const std::string sstr = m_lua["tostring"](arg.get<sol::object>());
            pMemory = _malloca(sizeof(uint64_t));

            *static_cast<uint64_t*>(pMemory) = RED4ext::FNV1a(sstr);
        }
        else if(pType == pCNameType)
        {
            const std::string sstr = m_lua["tostring"](arg.get<sol::object>());
            pMemory = _malloca(sizeof(CName));
            static_cast<CName*>(pMemory)->hash = RED4ext::FNV1a(sstr);
        }
        else
        {
            uint64_t hash = 0;
            pType->GetName(&hash);
            if (hash)
            {
                const std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
                aReturnMessage = "Function '" + acName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return make_object(m_lua, nullptr);
        }

        args[i].value = pMemory;
    }

    uint8_t buffer[1000]{0};

    CStackType result;
    if (hasReturnType)
    {
        result.value = buffer;
        result.type = *apFunc->returnType;
    }

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    auto* pScriptable = m_pHandle;
    if(pScriptable == nullptr)
        pScriptable = unk10->GetTypeInstance(m_pType);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, pScriptable, args.data(),
                                                          static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);

    const auto success = apFunc->Call(stack);
    if (!success)
        return make_object(m_lua, nullptr);

    if(hasReturnType)
    {
        auto luaResult = Scripting::ToLua(m_lua, result);
        if (luaResult)
            return luaResult;

        uint64_t hash = 0;
        (*apFunc->returnType)->GetName(&hash);
        if (hash)
        {
            const std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
            Overlay::Get().Log("Unhandled return type: " + typeName + " type : " + std::to_string((uint32_t)(*apFunc->returnType)->GetType()));
        }
    }

    return make_object(m_lua, true);
}
