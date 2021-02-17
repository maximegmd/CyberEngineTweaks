#include <stdafx.h>

#include "Sandbox.h"
#include "Scripting.h"

Sandbox::Sandbox(Scripting* apScripting, sol::environment aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_pScripting(apScripting)
    , m_env(apScripting->GetState().Get(), sol::create)
    , m_path(acRootPath)
{
    // copy base environment, do not set it as fallback, as it may cause globals to bleed into other things!
    for (const auto& cKV : aBaseEnvironment)
        m_env[cKV.first].set(cKV.second.as<sol::object>());
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath) const
{
    return m_pScripting->GetState().Get().script_file(acPath, m_env);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString) const
{
    return m_pScripting->GetState().Get().script(acString, m_env);
}

sol::environment& Sandbox::GetEnvironment()
{
    return m_env;
}

const std::filesystem::path& Sandbox::GetRootPath() const
{
    return m_path;
}
