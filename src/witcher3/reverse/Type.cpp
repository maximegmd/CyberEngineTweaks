#include <stdafx.h>

#include "Type.h"

#include <spdlog/fmt/fmt.h>

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

Type::Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, red3lib::IRTTIType* apClass)
    : m_pType(apClass)
    , m_lua(aView)
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
            auto view = cName.AsAnsiChar();
            return {view.begin(), view.end()};
        }
    }

    return "";
}

std::string Type::FunctionDescriptor(red3lib::CFunction* apFunc, bool aWithHashes) const
{
    std::stringstream ret;
    red3lib::CName typeName;
    TiltedPhoques::Vector<std::string> params;
    bool hasOutParams = false;

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
            descriptor.name = name.AsAnsiChar();
        }
    }

    return descriptor;
}

std::string Type::GameDump() const
{
    return "";
}

ClassType::ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, red3lib::IRTTIType* apClass)
    : Type(aView, apClass)
{
}

Type::Descriptor ClassType::Dump(bool aWithHashes) const
{
    auto descriptor = Type::Dump(aWithHashes);


    return descriptor;
}

sol::object ClassType::Index_Impl(const std::string& acName, sol::this_environment aThisEnv)
{
    auto* pClass = static_cast<red3lib::CClass*>(m_pType);

    if (!pClass)
        return sol::nil;

    auto* pHandle = GetHandle();

    if (pHandle)
    {
        return sol::nil;
    }

    auto result = Type::Index_Impl(acName, aThisEnv);

    if (result != sol::nil)
        return result;

    return sol::nil;
}

sol::object ClassType::NewIndex_Impl(const std::string& acName, sol::object aParam)
{
    auto* pClass = static_cast<red3lib::CClass*>(m_pType);

    if (!pClass)
        return sol::nil;

    auto* pHandle = GetHandle();

    if (pHandle)
    {
        bool success = false;
        //RTTIHelper::Get().SetProperty(pClass, pHandle, acName, aParam, success);

        if (success)
            return aParam;
    }

    return Type::NewIndex_Impl(acName, aParam);
}
