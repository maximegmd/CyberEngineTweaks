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
#include "SingletonReference.h"
#include "StrongReference.h"
#include "WeakReference.h"

#include <TiltedCore/ScratchAllocator.hpp>

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

    m_lua.new_usertype<StrongReference>("StrongReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &StrongReference::Index,
        sol::meta_function::new_index, &StrongReference::NewIndex);

    m_lua.new_usertype<WeakReference>("WeakReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &WeakReference::Index,
        sol::meta_function::new_index, &WeakReference::NewIndex);

    m_lua.new_usertype<SingletonReference>("SingletonReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &SingletonReference::Index,
        sol::meta_function::new_index, &SingletonReference::NewIndex);

    m_lua.new_usertype<GameOptions>("GameOptions",
        sol::meta_function::construct, sol::no_constructor,
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
    m_lua["GetSingleton"] = [this](const std::string& acName)
    {
        return this->GetSingletonHandle(acName);
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

    m_lua.do_file((Options::Get().Path / "scripts" / "autoexec.lua").string());
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

    if (pType == nullptr)
        return sol::nil;
    if (pType == pStringType)
        return make_object(aState, std::string(static_cast<RED4ext::REDreverse::CString*>(aResult.value)->ToString()));
    else if (pType == pInt32Type)
        return make_object(aState, *static_cast<int32_t*>(aResult.value));
    else if (pType == pBoolType)
        return make_object(aState, *static_cast<bool*>(aResult.value));
    else if (pType == pQuaternion)
        return make_object(aState, *static_cast<Quaternion*>(aResult.value));
    else if (pType == pCNameType)
        return make_object(aState, *static_cast<CName*>(aResult.value));
    else if (pType->GetType() == RED4ext::REDreverse::RTTIType::Handle)
    {
        const auto handle = *static_cast<StrongHandle*>(aResult.value);
        if (handle.handle)
            return make_object(aState, StrongReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::REDreverse::RTTIType::WeakHandle)
    {
        const auto handle = *static_cast<WeakHandle*>(aResult.value);
        if (handle.handle)
            return make_object(aState, WeakReference(aState, handle));
    }
    else
    {
        uint64_t hash = 0;
        pType->GetName(&hash);
        if (hash)
        {
            const std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
            Overlay::Get().Log("Unhandled return type: " + typeName + " type : " + std::to_string((uint32_t)pType->GetType()));
        }
    }

    return sol::nil;
}

RED4ext::REDreverse::CScriptableStackFrame::CStackType Scripting::ToRED(sol::object aObject, RED4ext::REDreverse::CRTTIBaseType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    static auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));
    static auto* pCNameType = pRtti->GetType(RED4ext::FNV1a("CName"));
    static auto* pInt32Type = pRtti->GetType(RED4ext::FNV1a("Int32"));
    static auto* pBoolType = pRtti->GetType(RED4ext::FNV1a("Bool"));
    static auto* pQuaternion = pRtti->GetType(RED4ext::FNV1a("Quaternion"));

    RED4ext::REDreverse::CScriptableStackFrame::CStackType result;

    sol::state_view v(aObject.lua_state());

    if (apRtti)
    {
        result.type = apRtti;

        if (apRtti == pStringType)
        {
            const std::string sstr = v["tostring"](aObject);
            result.value = apAllocator->New<RED4ext::REDreverse::CString>(sstr.c_str());
        }
        else if (apRtti == pInt32Type)
            result.value = apAllocator->New<int32_t>(aObject.as<int32_t>());
        else if (apRtti == pBoolType)
            result.value = apAllocator->New<bool>(aObject.as<bool>());
        else if (apRtti == pQuaternion)
            result.value = apAllocator->New<Quaternion>(aObject.as<Quaternion>());
        else if (apRtti == pCNameType)
        {
            const std::string sstr = v["tostring"](aObject);
            result.value = apAllocator->New<CName>(RED4ext::FNV1a(sstr));
        }
    }

    return result;
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

sol::object Scripting::GetSingletonHandle(const std::string& acName)
{
    auto itor = m_singletons.find(acName);
    if (itor != std::end(m_singletons))
        return make_object(m_lua, itor->second);

    auto* pRtti = RED4ext::REDreverse::CRTTISystem::Get();
    auto* pType = pRtti->GetType<RED4ext::REDreverse::CClass*>(RED4ext::FNV1a(acName));
    if (!pType)
    {
        Overlay::Get().Log("Type '" + acName + "' not found or is not initialized yet.");
        return sol::nil;
    }

    auto result = m_singletons.emplace(std::make_pair(acName, SingletonReference{ m_lua, pType }));
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

    RED4ext::REDreverse::CBaseFunction* pFunc = pRtti->GetGlobalFunction(RED4ext::FNV1a(aFuncName.c_str()));

    static const auto hashcpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");
    static const auto hashGameInstance = RED4ext::FNV1a("ScriptGameInstance");

    auto* pPlayerSystem = pRtti->GetType<RED4ext::REDreverse::CClass*>(hashcpPlayerSystem);
    auto* gameInstanceType = pRtti->GetType<RED4ext::REDreverse::CClass*>(hashGameInstance);

    if (!pFunc)
    {
        pFunc = gameInstanceType->GetFunction(RED4ext::FNV1a(aFuncName.c_str()));
        if (!pFunc)
        {
            aReturnMessage = "Function '" + aFuncName + "' not found or is not a global.";

            return sol::nil;
        }
    }

    if (pFunc->params.size - 1 != aArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' expects " +
            std::to_string(pFunc->params.size - 1) + " parameters, not " +
            std::to_string(aArgs.size()) + ".";

        return sol::nil;
    }

    const auto* engine = RED4ext::REDreverse::CGameEngine::Get();
    auto* unk10 = engine->framework->gameInstance;

    using CStackType = RED4ext::REDreverse::CScriptableStackFrame::CStackType;
    std::vector<CStackType> args(aArgs.size() + 1);

    args[0].type = *pFunc->params.types[0];
    args[0].value = &unk10;

    // 8KB should cut it
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    for (auto i = 0ull; i < aArgs.size(); ++i)
    {
        args[i + 1ull] = ToRED(aArgs[i].get<sol::object>(), *pFunc->params.types[i + 1ull], &s_scratchMemory);

        if(!args[i + 1ull].value)
        {
            auto* pType = *pFunc->params.types[i + 1];

            uint64_t hash = 0;
            pType->GetName(&hash);
            if (hash)
            {
                const auto typeName = RED4ext::REDreverse::CName::ToString(hash);
                aReturnMessage = "Function '" + aFuncName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return sol::nil;
        }
    }

    const bool hasReturnType = (pFunc->returnType) != nullptr && (*pFunc->returnType) != nullptr;

    uint8_t buffer[1000]{ 0 };
    CStackType result;
    if (hasReturnType)
    {
        result.value = &buffer;
        result.type = *pFunc->returnType;
    }

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    const auto* pScriptable = unk10->GetTypeInstance(pPlayerSystem);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, pScriptable, args.data(),
                                                          static_cast<uint32_t>(args.size()),  hasReturnType ? &result : nullptr, 0);
    const auto success = pFunc->Call(stack);
    if (!success)
        return sol::nil;

    if(hasReturnType)
        return ToLua(m_lua, result);

    return make_object(m_lua, true);
}
