#include <stdafx.h>
#include <spdlog/fmt/fmt.h>

#include "Type.h"

#include "RTTIHelper.h"


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

Type::Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::CBaseRTTIType* apClass)
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
        const auto cName = m_pType->GetName();
        if (!cName.IsNone())
        {
            return cName.ToString();
        }
    }

    return "";
}

std::string Type::FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const
{
    std::stringstream ret;
    RED4ext::CName typeName;
    TiltedPhoques::Vector<std::string> params;
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
        typeName = param->type->GetName();
        params.push_back(fmt::format("{}{}: {}", param->flags.isOptional ? "[opt] " : "",
                                    param->name.ToString(), typeName.ToString()));
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
        typeName = apFunc->returnType->type->GetName();
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

            typeName = param->type->GetName();
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
        const auto name = m_pType->GetName();
        if (!name.IsNone())
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
            m_pType->ToString(handle, str);
        }
    }

    return str.c_str();
}

ClassType::ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                     RED4ext::CBaseRTTIType* apClass)
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
            const auto* cpProperty = type->props[i];
            const auto cName = cpProperty->type->GetName();

            descriptor.properties.push_back(cpProperty->name.ToString() + std::string(": ") + cName.ToString());
        }

        type = type->parent && type->parent->GetType() == RED4ext::ERTTIType::Class ? type->parent : nullptr;
    }

    return descriptor;
}

sol::object ClassType::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    auto* pClass = static_cast<RED4ext::CClass*>(m_pType);

    if (!pClass)
        return sol::nil;

    auto* pHandle = GetHandle();

    if (pHandle)
    {
        bool success = false;
        auto result = RTTIHelper::Get().GetProperty(pClass, pHandle, acName, success);

        if (success)
            return result;
    }

    auto result = Type::Index_Impl(acName, aThisEnv);

    if (result != sol::nil)
        return result;

    auto func = RTTIHelper::Get().ResolveFunction(pClass, acName, pHandle != nullptr);

    if (!func)
    {
        const sol::environment cEnv = aThisEnv;
        std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->warn("Warning: {} not found in {}.", acName, GetName());
        return sol::nil;
    }

    return Type::NewIndex(acName, std::move(func));
}

sol::object ClassType::NewIndex_Impl(const std::string& acName, sol::object aParam)
{
    auto* pClass = static_cast<RED4ext::CClass*>(m_pType);

    if (!pClass)
        return sol::nil;

    auto* pHandle = GetHandle();

    if (pHandle)
    {
        bool success = false;
        RTTIHelper::Get().SetProperty(pClass, pHandle, acName, aParam, success);

        if (success)
            return aParam;
    }

    return Type::NewIndex_Impl(acName, aParam);
}

UnknownType::UnknownType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                         RED4ext::CBaseRTTIType* apClass,
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
