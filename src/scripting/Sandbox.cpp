#include "stdafx.h"

#include "Sandbox.h"
#include "Scripting.h"

Sandbox::Sandbox(uint64_t aId, Scripting* apScripting, sol::table aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_id(aId)
    , m_pScripting(apScripting)
    , m_env(apScripting->GetLockedState().Get(), sol::create, aBaseEnvironment)
    , m_path(acRootPath)
{
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath) const
{
    return m_pScripting->GetLockedState().Get().script_file(acPath, m_env, sol::load_mode::text);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString) const
{
    return m_pScripting->GetLockedState().Get().script(acString, m_env, sol::detail::default_chunk_name(), sol::load_mode::text);
}

uint64_t Sandbox::GetId() const
{
    return m_id;
}

sol::environment& Sandbox::GetEnvironment()
{
    return m_env;
}

const std::filesystem::path& Sandbox::GetRootPath() const
{
    return m_path;
}
