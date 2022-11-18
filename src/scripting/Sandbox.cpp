#include <stdafx.h>

#include "Sandbox.h"
#include "Scripting.h"

Sandbox::Sandbox(Scripting& aScripting, uint64_t aId, sol::table aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_scripting(aScripting)
    , m_id(aId)
    , m_env(aScripting.GetLockedState().Get(), sol::create, aBaseEnvironment)
    , m_path(acRootPath)
{
}

Sandbox::Sandbox(Sandbox&& aOther) noexcept
    : m_scripting(aOther.m_scripting)
    , m_id(aOther.m_id)
    , m_env(std::move(aOther.m_env))
    , m_path(std::move(aOther.m_path))
{
}

Sandbox& Sandbox::operator=(Sandbox&& aOther) noexcept
{
    if (this == &aOther)
        return *this;

    m_id = aOther.m_id;
    m_env = aOther.m_env;
    m_path = aOther.m_path;

    return *this;
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath) const
{
    return m_scripting.GetLockedState().Get().script_file(acPath, m_env, sol::load_mode::text);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString) const
{
    return m_scripting.GetLockedState().Get().script(acString, m_env, sol:: detail::default_chunk_name(), sol::load_mode::text);
}

uint64_t Sandbox::GetId() const
{
    return m_id;
}

sol::environment& Sandbox::GetEnvironment()
{
    return m_env;
}

const sol::environment& Sandbox::GetEnvironment() const
{
    return m_env;
}

const std::filesystem::path& Sandbox::GetRootPath() const
{
    return m_path;
}
