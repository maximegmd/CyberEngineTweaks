#include <stdafx.h>

#include "Sandbox.h"
#include "Scripting.h"

#include <Utils.h>

Sandbox::Sandbox(uint64_t aId, Scripting* apScripting, sol::environment aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_pScripting(apScripting)
    , m_env(apScripting->GetState().Get(), sol::create)
    , m_path(acRootPath)
    , m_id(aId)
{
    // copy base environment, do not set it as fallback, as it may cause globals to bleed into other things!
    sol::state_view sv = apScripting->GetState().Get();
    for (const auto& cKV : aBaseEnvironment)
        m_env[cKV.first] = DeepCopySolObject(cKV.second, sv);

    // set global fallback table for the environment
    sol::table metatable(sv, sol::create);
    metatable[sol::meta_function::index] = sv.get<sol::table>(apScripting->GetGlobalName());
    m_env[sol::metatable_key] = metatable;
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath) const
{
    auto lock = m_pScripting->GetState();
    return m_pScripting->GetState().Get().script_file(acPath, m_env, sol::load_mode::text);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString) const
{
    auto lock = m_pScripting->GetState();
    return m_pScripting->GetState().Get().script(acString, m_env, sol:: detail::default_chunk_name(), sol::load_mode::text);
}

uint64_t Sandbox::GetId() const
{
    return m_id;
}

sol::environment& Sandbox::GetEnvironment()
{
    return m_env;
}

sol::table& Sandbox::GetImGui()
{
    return m_imgui;
}

const std::filesystem::path& Sandbox::GetRootPath() const
{
    return m_path;
}
