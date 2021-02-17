#include <stdafx.h>
#include <spdlog/fmt/fmt.h>

#include "Type.h"

#include "scripting/Scripting.h"


std::string Type::Descriptor::ToString() const
{
    std::string result;
    result += "{\n\tname: " + name + ",\n\tfunctions: {\n";
    for (auto& function : functions)
    {
        result += "\t\t" + function + ",\n";
    }
    result += "\t},\n\tstaticFunctions: {\n";
    for (auto& function : staticFunctions)
    {
        result += "\t\t" + function + ",\n";
    }
    result += "\t},\n\tproperties: {\n";
    for (auto& property : properties)
    {
        result += "\t\t" + property + ",\n";
    }
    result += "\t}\n}";

    return result;
}

Type::Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::IRTTIType* apClass)
    : m_lua(aView)
    , m_pType(apClass)
{
}

sol::object Type::Index(const std::string& acName, sol::this_environment aThisEnv)
{
    return Index_Impl(acName, aThisEnv);
}

sol::object Type::NewIndex(const std::string& acName, sol::object aParam)
{
    return NewIndex_Impl(acName, aParam);
}

sol::object Type::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    if (const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return sol::nil;
}

sol::object Type::NewIndex_Impl(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = std::move(aParam);
    return property;
}

std::string Type::GetName() const
{
    if (m_pType)
    {
        RED4ext::CName name;
        m_pType->GetName(name);
        if (!name.IsEmpty())
        {
            return name.ToString();
        }
    }

    return "";
}

std::string Type::FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const
{
    std::stringstream ret;
    RED4ext::CName typeName;
    std::vector<std::string> params;
    bool hasOutParams = false;

    // name2 seems to be a cleaner representation of the name
    // for example, name would be "DisableFootstepAudio;Bool", and name2 is just"DisableFootstepAudio"
    const std::string funcName2 = apFunc->shortName.ToString();

    ret << funcName2 << "(";

    for (auto i = 0u; i < apFunc->params.size; ++i)
    {
        auto* param = apFunc->params[i];

        if (param->flags.isOut)
        {
            // 'out' param, for returning additional data
            // we hide these here so we can display them in the return types
            hasOutParams = true;
            continue;
        }
        param->type->GetName(typeName);
        params.push_back(param->name.ToString() + std::string(": ") + typeName.ToString());
    }

    for (auto i = 0u; i < params.size(); ++i)
    {
        ret << params[i];
        if (i < params.size() - 1)
        {
            ret << ", ";
        }
    }

    ret << ")";

    const bool hasReturnType = (apFunc->returnType) != nullptr && (apFunc->returnType->type) != nullptr;

    params.clear();

    if (hasReturnType)
    {
        apFunc->returnType->type->GetName(typeName);
        params.push_back(typeName.ToString());
    }

    if (hasOutParams)
    {
        for (auto i = 0u; i < apFunc->params.size; ++i)
        {
            auto* param = apFunc->params[i];

            if (!param->flags.isOut)
            {
                // ignone non-out params cause we've dealt with them above
                continue;
            }

            param->type->GetName(typeName);
            params.push_back(param->name.ToString() + std::string(": ") + typeName.ToString());
        }

    }

    if (params.size() > 0)
    {
        ret << " => (";

        for (auto i = 0; i < params.size(); ++i)
        {
            ret << params[i];
            if (i < params.size() - 1)
            {
                ret << ", ";
            }
        }

        ret << ")";
    }


    if (aWithHashes)
    {
        const std::string funcHashes = "Hash:(" + fmt::format("{:016x}", apFunc->fullName.hash) + ") / ShortName:(" + apFunc->shortName.ToString() + ") Hash:(" + fmt::format("{:016x}", apFunc->shortName.hash) + ")";
        ret << " # " << funcHashes;
    }

    return ret.str();
}

Type::Descriptor Type::Dump(bool aWithHashes) const
{
    Descriptor descriptor;

    if (m_pType)
    {
        RED4ext::CName name;
        m_pType->GetName(name);
        if (!name.IsEmpty())
        {
            descriptor.name = name.ToString();
        }
    }

    return descriptor;
}

std::string Type::GameDump()
{
    RED4ext::CString str("");
    if (m_pType)
    {
        auto handle = GetHandle();
        if (handle)
        {
            m_pType->GetDebugString(handle, str);
        }
    }

    return str.c_str();
}

sol::variadic_results Type::Execute(RED4ext::CBaseFunction* apFunc, const std::string& acName, sol::variadic_args aArgs, std::string& aReturnMessage)
{

    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    TiltedPhoques::Vector<RED4ext::CStackType> args(apFunc->params.size);

    int32_t minArgs = 0;
    for (auto i = 0; i < apFunc->params.size; ++i)
    {
        if (!apFunc->params[i]->flags.isOut && !apFunc->params[i]->flags.isOptional)
        {
            minArgs++;
        }
    }

    if (minArgs > aArgs.size())
    {
        aReturnMessage = "Function '" + acName + "' requires at least " + std::to_string(minArgs) + " parameter(s).";
        return {};
    }

    for (auto i = 0u; i < apFunc->params.size; ++i)
    {
        if (apFunc->params[i]->flags.isOut) // Deal with out params
        {
            args[i] = Scripting::ToRED(sol::nil, apFunc->params[i]->type, &s_scratchMemory);
        }
        else if (i < aArgs.size())
        {
            args[i] = Scripting::ToRED(aArgs[i].get<sol::object>(), apFunc->params[i]->type, &s_scratchMemory);
        }
        else if (apFunc->params[i]->flags.isOptional) // Deal with optional params
        {
            args[i].value = nullptr;
        }

        if (!args[i].value && !apFunc->params[i]->flags.isOptional)
        {
            auto* pType = apFunc->params[i]->type;

            RED4ext::CName hash;
            pType->GetName(hash);
            if (!hash.IsEmpty())
            {
                std::string typeName = hash.ToString();
                aReturnMessage = "Function '" + acName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return {};
        }
    }

    const bool hasReturnType = (apFunc->returnType) != nullptr && (apFunc->returnType->type) != nullptr;

    uint8_t buffer[1000]{ 0 };
    RED4ext::CStackType result;
    if (hasReturnType)
    {
        result.value = buffer;
        result.type = apFunc->returnType->type;
    }

    // Needs more investigation, but if you do something like GetSingleton('inkMenuScenario'):GetSystemRequestsHandler()
    // which actually does not need the 'this' object, you can trick it into calling it as long as you pass something for the handle
    // GetSingleton('inkMenuScenario') would return nullptr handle because its not in the GI table
    auto handle = GetHandle();
    if (!handle)
    {
        const auto* engine = RED4ext::CGameEngine::Get();
        handle = reinterpret_cast<RED4ext::ScriptInstance>(engine->framework->gameInstance); // Not actually derived from IScriptable but still an "Instance"
    }

    RED4ext::CStack stack(handle, args.data(), args.size(), hasReturnType ? &result : nullptr, 0);

    const auto success = apFunc->Execute(&stack);
    if (!success)
        return {};

    sol::variadic_results results;

    auto state = m_lua.Lock();

    if (hasReturnType)
        results.push_back(Scripting::ToLua(state, result));

    for (auto i = 0; i < apFunc->params.size; ++i)
    {
        if (!apFunc->params[i]->flags.isOut)
            continue;

        results.push_back(Scripting::ToLua(state, args[i]));
    }

    return results;
}


ClassType::ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                     RED4ext::IRTTIType* apClass)
    : Type(aView, apClass)
{

}

Type::Descriptor ClassType::Dump(bool aWithHashes) const
{
    auto descriptor = Type::Dump(aWithHashes);

    auto* type = static_cast<RED4ext::CClass*>(m_pType);
    while (type)
    {
        std::string name = type->name.ToString();
        for (auto i = 0u; i < type->funcs.size; ++i)
        {
            auto* pFunc = type->funcs[i];
            std::string funcDesc = FunctionDescriptor(pFunc, aWithHashes);
            descriptor.functions.push_back(funcDesc);
        }

        for (auto i = 0u; i < type->staticFuncs.size; ++i)
        {
            auto* pFunc = type->staticFuncs[i];
            std::string funcDesc = FunctionDescriptor(pFunc, aWithHashes);
            descriptor.staticFunctions.push_back(funcDesc);
        }

        for (auto i = 0u; i < type->props.size; ++i)
        {
            auto* pProperty = type->props[i];
            RED4ext::CName name;
            pProperty->type->GetName(name);

            descriptor.properties.push_back(pProperty->name.ToString() + std::string(": ") + name.ToString());
        }

        type = type->parent && type->parent->GetType() == RED4ext::ERTTIType::Class ? type->parent : nullptr;
    }

    return descriptor;
}

sol::object ClassType::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    auto pType = static_cast<RED4ext::CClass*>(m_pType);
    auto handle = GetHandle();
    if (pType && handle)
    {
        auto prop = pType->GetProperty(acName.c_str());
        if (prop)
        {
            auto value = prop->GetValue<uintptr_t*>(handle);
            RED4ext::CStackType stackType(prop->type, value);
            auto state = m_lua.Lock();
            return Scripting::ToLua(state, stackType);
        }
    }

    auto res = Type::Index_Impl(acName, aThisEnv);
    if (res != sol::nil)
    {
        return res;
    }

    if (!pType)
    {
        return sol::nil;
    }

    auto* pFunc = pType->GetFunction(RED4ext::FNV1a(acName.c_str()));
    if (!pFunc)
    {
        // Search the function table if it isn't found, the above function only searches by ShortName so overloads are not found
        for (uint32_t i = 0; i < pType->funcs.size; ++i)
        {
            if (pType->funcs.entries[i]->fullName.hash == RED4ext::FNV1a(acName.c_str()))
            {
                pFunc = static_cast<RED4ext::CClassFunction*>(pType->funcs.entries[i]);
                break;
            }
        }

        if (!pFunc)
        {
            const sol::environment cEnv = aThisEnv;
            std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Function '{}' not found in system '{}'.", acName, GetName());
            return sol::nil;
        }
    }

    auto state = m_lua.Lock();

    auto obj = make_object(state.Get(), [pFunc, name = acName](Type* apType, sol::variadic_args aArgs,
                                                       sol::this_environment aThisEnv, sol::this_state L)
    {
        std::string result;
        auto funcRet = apType->Execute(pFunc, name, aArgs, result);
        if (!result.empty())
        {
            const sol::environment cEnv = aThisEnv;
            std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Error: {}", result);
        }
        return funcRet;
    });

    return Type::NewIndex_Impl(acName, std::move(obj));
}

sol::object ClassType::NewIndex_Impl(const std::string& acName, sol::object aParam)
{
    auto pType = static_cast<RED4ext::CClass*>(m_pType);
    auto handle = GetHandle();
    if (handle)
    {
        auto prop = pType->GetProperty(acName.c_str());
        if (prop)
        {
            static thread_local TiltedPhoques::ScratchAllocator s_propScratchMemory(1 << 10);
            struct ResetAllocator
            {
                ~ResetAllocator()
                {
                    s_propScratchMemory.Reset();
                }
            };
            ResetAllocator ___allocatorReset;
            RED4ext::CStackType stackType = Scripting::ToRED(aParam, prop->type, &s_propScratchMemory);
            prop->SetValue<uintptr_t*>(handle, static_cast<uintptr_t*>(stackType.value));
            return aParam;
        }
    }

    return Type::NewIndex_Impl(acName, aParam);
}

UnknownType::UnknownType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                         RED4ext::IRTTIType* apClass,
                         RED4ext::ScriptInstance apInstance)
    : Type(aView, apClass)
{
    // Hack for now until we use their allocators
    m_pInstance = std::make_unique<uint8_t[]>(apClass->GetSize());
    memcpy(m_pInstance.get(), apInstance, apClass->GetSize());
}

Type::Descriptor UnknownType::Dump(bool aWithHashes) const
{
    auto descriptor = Type::Dump(aWithHashes);
    descriptor.name += " Unknown Type " + std::to_string(static_cast<uint32_t>(m_pType->GetType()));
    return descriptor;
}
