#include <stdafx.h>

#include "Type.h"

#include "scripting/Scripting.h"
#include "overlay/Overlay.h"


std::string Type::Descriptor::ToString() const
{
    std::string result;
    result += "{\n\tname: " + name + ",\n\tfunctions: {\n";
    for (auto& function : functions)
    {
        result += "\t\t" + function + ",\n";
    }
    result += "},\nproperties: {\n";
    for (auto& property : properties)
    {
        result += "\t\t" + property + ",\n";
    }
    result += "\t}\n}";

    return result;
}

Type::Type(sol::state_view aView, RED4ext::REDreverse::CClass* apClass)
    : m_lua(std::move(aView))
    , m_pType(apClass)
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
    if (!m_pType)
    {
        return sol::nil;
    }

    auto* pFunc = m_pType->GetFunction(RED4ext::FNV1a(acName));
    if(!pFunc)
    {
        Overlay::Get().Log("Function '" + acName + "' not found in system '" + GetName() + "'.");
        return sol::nil;
    }

    auto obj = make_object(m_lua, [pFunc, name = acName](Type* apType, sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        auto funcRet = apType->Execute(pFunc, name, args, env, L, result);
        if(!result.empty())
        {
            Overlay::Get().Log("Error: " + result);
        }
        return funcRet;
    });

    return NewIndex(acName, std::move(obj));
}

std::string Type::GetName() const
{
    if (m_pType)
    {
        uint64_t name;
        m_pType->GetName(&name);
        if (name)
        {
            return RED4ext::REDreverse::CName::ToString(name);
        }
    }

    return "";
}

Type::Descriptor Type::Dump() const
{
    Descriptor descriptor;

    if(m_pType)
    {
        descriptor.name = RED4ext::REDreverse::CName::ToString(m_pType->nameHash);

        for (auto i = 0u; i < m_pType->functions.size; ++i)
        {
            auto* pFunc = m_pType->functions.arr[i];
            std::string funcName = RED4ext::REDreverse::CName::ToString(pFunc->nameHash);
            descriptor.functions.push_back(funcName);
        }

        for (auto i = 0u; i < m_pType->properties.size; ++i)
        {
            auto* pProperty = m_pType->properties.arr[i];
            uint64_t name = 0;
            pProperty->type->GetName(&name);
            
            std::string propName = std::string(RED4ext::REDreverse::CName::ToString(pProperty->name)) + " : " + RED4ext::REDreverse::CName::ToString(name);
            descriptor.properties.push_back(propName);
        }
    }

    return descriptor;
}

sol::object Type::Execute(RED4ext::REDreverse::CClassFunction* apFunc, const std::string& acName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    std::vector<RED4ext::REDreverse::CScriptableStackFrame::CStackType> args(apFunc->params.size);

    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    for (auto i = 0u; i < apFunc->params.size; ++i)
    {
        if ((apFunc->params.arr[i]->flag & 0x200) != 0) // Deal with out params
        {
            args[i] = Scripting::ToRED(sol::nil, apFunc->params.arr[i]->type, &s_scratchMemory);
        }
        else if (aArgs.size() > i)
        {
            args[i] = Scripting::ToRED(aArgs[i].get<sol::object>(), apFunc->params.arr[i]->type, &s_scratchMemory);
        }
        else if((apFunc->params.arr[i]->flag & 0x400) != 0) // Deal with optional params
        {
            args[i].value = nullptr;
        }

        if (!args[i].value && (apFunc->params.arr[i]->flag & 0x1) == 0)
        {
            auto* pType = apFunc->params.arr[i]->type;

            uint64_t hash = 0;
            pType->GetName(&hash);
            if (hash)
            {
                std::string typeName = RED4ext::REDreverse::CName::ToString(hash);
                aReturnMessage = "Function '" + acName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
            }

            return sol::nil;
        }
    }

    const bool hasReturnType = (apFunc->returnType) != nullptr && (*apFunc->returnType) != nullptr;

    uint8_t buffer[1000]{0};
    RED4ext::REDreverse::CScriptableStackFrame::CStackType result;
    if (hasReturnType)
    {
        result.value = buffer;
        result.type = *apFunc->returnType;
    }

    std::aligned_storage_t<sizeof(RED4ext::REDreverse::CScriptableStackFrame), alignof(RED4ext::REDreverse::CScriptableStackFrame)> stackStore;
    auto* stack = reinterpret_cast<RED4ext::REDreverse::CScriptableStackFrame*>(&stackStore);

    RED4ext::REDreverse::CScriptableStackFrame::Construct(stack, GetHandle(), args.data(),
                                                          static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);

    const auto success = apFunc->Call(stack);
    if (!success)
        return sol::nil;

    std::vector<sol::object> returns;

    if (hasReturnType)
        returns.push_back(Scripting::ToLua(m_lua, result));

    for (auto i = 0; i < apFunc->params.size; ++i)
    {
        if ((apFunc->params.arr[i]->flag & 0x200) == 0)
            continue;

        returns.push_back(Scripting::ToLua(m_lua, args[i]));
    }

    if (returns.empty())
        return sol::nil;
    if (returns.size() == 1)
        return returns[0];

    return make_object(m_lua, returns);
}
