#include <stdafx.h>

#include "Sandbox.h"

Sandbox::Sandbox(sol::state_view aStateView, sol::environment aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create)
    , m_path(acRootPath)
{
    // copy base environment, do not set it as fallback, as it may cause globals to bleed into other things!
    for (const auto& cKV : aBaseEnvironment)
        m_env[cKV.first].set(cKV.second.as<sol::object>());
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath)
{
    return m_lua.script_file(acPath, m_env);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString)
{
    return m_lua.script(acString, m_env);
}

sol::state_view& Sandbox::GetStateView()
{
    return m_lua;
}

sol::environment& Sandbox::GetEnvironment()
{
    return m_env;
}

const std::filesystem::path& Sandbox::GetRootPath() const
{
    return m_path;
}
