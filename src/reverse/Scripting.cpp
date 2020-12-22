#include "Scripting.h"
#include "RED4ext/Scripting.hpp"

#include <algorithm>
#include <vector>
#include <spdlog/spdlog.h>

#include "BasicTypes.h"
#include "Options.h"
#include "Type.h"
#include "RED4ext/REDreverse/CString.hpp"
#include "overlay/Overlay.h"
#include "RED4ext/REDreverse/CName.hpp"

#include "GameOptions.h"

Scripting::Scripting()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);

    m_lua.new_usertype<Scripting>("__Game",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Scripting::Index,
        sol::meta_function::new_index, &Scripting::NewIndex);

    m_lua.new_usertype<Type>("__Type",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Type::Index,
        sol::meta_function::new_index, &Type::NewIndex);

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
        "Dump", &GameOptions::Dump,
        "List", &GameOptions::List);

    m_lua.new_usertype<Quaternion>("Quaternion",
        sol::meta_function::to_string, &Quaternion::ToString,
        "x", sol::property(&Quaternion::x),
        "y", sol::property(&Quaternion::y),
        "z", sol::property(&Quaternion::z),
        "w", sol::property(&Quaternion::w));

    m_lua.new_usertype<CName>("CName",
        sol::meta_function::to_string, &CName::ToString,
        "hash", sol::property(&CName::hash));

    m_lua["Game"] = this;
    m_lua["CreateSingletonHandle"] = [this](const std::string& acName)
    {
        return this->CreateSingletonHandle(acName);
    };

    m_lua["CreateHandle"] = [this](StrongHandle* apHandle)
    {
        auto* pType = apHandle->handle->GetClass();
        if(pType)
        {
            uint64_t hash = 0;
            pType->GetName(&hash);
            if(hash)
            {
                const auto type = RED4ext::REDreverse::CName::ToString(hash);
                return this->CreateHandle(type, apHandle->handle);
            }
        }

        return make_object(m_lua, nullptr);
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

sol::object Scripting::ToLua(sol::state_view aState, RED4ext::REDreverse::CScriptableStackFrame::CStackType& aResult)
{
    static auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));
    static auto* pCNameType = pRtti->GetType(RED4ext::FNV1a("CName"));
    static auto* pInt32Type = pRtti->GetType(RED4ext::FNV1a("Int32"));
    static auto* pBoolType = pRtti->GetType(RED4ext::FNV1a("Bool"));
    static auto* pQuaternion = pRtti->GetType(RED4ext::FNV1a("Quaternion"));

    auto* pType = aResult.type;

    if (pType == pStringType)
        return make_object(aState, std::string(static_cast<RED4ext::REDreverse::CString*>(aResult.value)->ToString()));
    if (pType == pInt32Type)
        return make_object(aState, *static_cast<int32_t*>(aResult.value));
    if (pType == pBoolType)
        return make_object(aState, *static_cast<bool*>(aResult.value));
    if (pType == pQuaternion)
        return make_object(aState, *static_cast<Quaternion*>(aResult.value));
    if (pType == pCNameType)
        return make_object(aState, *static_cast<CName*>(aResult.value));
    if (pType->GetType() == RED4ext::REDreverse::RTTIType::Handle)
        return make_object(aState, *static_cast<StrongHandle*>(aResult.value));

    return make_object(aState, nullptr);
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

sol::object Scripting::CreateSingletonHandle(const std::string& acName)
{
    return CreateHandle(acName, nullptr);
}

sol::object Scripting::CreateHandle(const std::string& acName, RED4ext::REDreverse::Scripting::IScriptable* apHandle)
{
    auto itor = m_systems.find(acName);
    if (itor != std::end(m_systems))
        return make_object(m_lua, itor->second);

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    auto* pType = pRtti->GetType<RED4ext::REDreverse::CClass*>(RED4ext::FNV1a(acName));
    if (!pType)
    {
        Overlay::Get().Log("Type '" + acName + "' not found or is not initialized yet.");
        return make_object(m_lua, nullptr);
    }

    auto result = m_systems.emplace(std::make_pair(acName, Type{ m_lua, pType, acName, apHandle }));
    return make_object(m_lua, result.first->second);
}

sol::protected_function Scripting::InternalIndex(const std::string& acName)
{
    const sol::state_view state(m_lua);
    auto obj = make_object(state, [this, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        auto code = this->Execute(name, args, env, L, result);
        if(!code)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return code;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    auto* pFunc = pRtti->GetGlobalFunction(RED4ext::FNV1a(aFuncName.c_str()));
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));
    static auto* pCNameType = pRtti->GetType(RED4ext::FNV1a("CName"));
    static auto* pInt32Type = pRtti->GetType(RED4ext::FNV1a("Int32"));
    static auto* pBoolType = pRtti->GetType(RED4ext::FNV1a("Bool"));

    if (!pFunc)
    {
        aReturnMessage = "Function '" + aFuncName + "' not found or is not a global.";

        return make_object(m_lua, nullptr);
    }

    if (pFunc->params.size - 1 != aArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' expects " +
            std::to_string(pFunc->params.size - 1) + " parameters, not " +
            std::to_string(aArgs.size()) + ".";

        return make_object(m_lua, nullptr);
    }

    auto engine = RED4ext::REDreverse::CGameEngine::Get();
    auto unk10 = engine->framework->unk10;

    using CStackType = RED4ext::REDreverse::CScriptableStackFrame::CStackType;
    std::vector<CStackType> args(aArgs.size() + 1);
    args[0].type = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[0]);
    args[0].value = &unk10;

    const bool hasReturnType = (pFunc->returnType) != nullptr && (*pFunc->returnType) != nullptr && (*pFunc->returnType)->GetType() == RED4ext::REDreverse::RTTIType::Handle;

    for (auto i = 0; i < aArgs.size(); ++i)
    {
        auto arg = aArgs[i];

        auto* pType = *reinterpret_cast<RED4ext::REDreverse::CRTTIBaseType**>(pFunc->params.unk0[i + 1]);
        args[i + 1].type = pType;

        void* pMemory = nullptr;

        if (pType == pStringType)
        {
            const std::string sstr = m_lua["tostring"](arg.get<sol::object>());
            pMemory = _malloca(sizeof(RED4ext::REDreverse::CString));

            new (pMemory) RED4ext::REDreverse::CString{ sstr.c_str() };
        }
        else if (pType == pInt32Type)
        {
            pMemory = _malloca(sizeof(int32_t));
            *static_cast<int32_t*>(pMemory) = arg.get<int32_t>();
        }
        else if (pType == pBoolType)
        {
            pMemory = _malloca(sizeof(bool));
            *static_cast<bool*>(pMemory) = arg.get<bool>();
        }
        else if (pType == pCNameType)
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
                std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
                aReturnMessage = "Function '" + aFuncName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return make_object(m_lua, nullptr);
        }

        args[i + 1].value = pMemory;

    }


    uint8_t buffer[1000]{ 0 };

    CStackType result;
    if (hasReturnType)
    {
        result.value = &buffer;
        result.type = *pFunc->returnType;
    }

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    static const auto cpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");

    const auto* type = pRtti->GetType(cpPlayerSystem);

    auto pScriptable = unk10->GetTypeInstance(type);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, pScriptable, args.data(),
                                                          static_cast<uint32_t>(args.size()),  hasReturnType ? &result : nullptr, 0);
    const auto success = pFunc->Call(stack);
    if (!success)
        return make_object(m_lua, nullptr);

    if(hasReturnType)
    {
        auto luaResult = ToLua(m_lua, result);
        if (luaResult)
            return luaResult;

        uint64_t hash = 0;
        (*pFunc->returnType)->GetName(&hash);
        if (hash)
        {
            const std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
            Overlay::Get().Log("Unhandled return type: " + typeName + " type : " + std::to_string((uint32_t)(*pFunc->returnType)->GetType()));
        }
    }

    return make_object(m_lua, true);
}
