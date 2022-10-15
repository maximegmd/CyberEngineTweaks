#include <stdafx.h>

#include "Sandbox.h"
#include "Scripting.h"

#include <Utils.h>

Sandbox::Sandbox(uint64_t aId, Scripting* apScripting, sol::environment aBaseEnvironment, const std::filesystem::path& acRootPath)
    : m_id(aId)
    , m_pScripting(apScripting)
    , m_env(apScripting->GetLockedState().Get(), sol::create)
    , m_path(acRootPath)
{
    // copy base environment, do not set it as fallback, as it may cause globals to bleed into other things!
    const auto& luaState = apScripting->GetLockedState().Get();
    for (const auto& cKV : aBaseEnvironment)
        m_env[cKV.first] = DeepCopySolObject(cKV.second, luaState);

    // set global fallback table for the environment
    sol::table metatable(luaState, sol::create);
    metatable[sol::meta_function::index] = luaState.get<sol::table>(apScripting->GetGlobalName());
    m_env[sol::metatable_key] = metatable;
}

sol::protected_function_result Sandbox::ExecuteFile(const std::string& acPath) const
{
    return m_pScripting->GetLockedState().Get().script_file(acPath, m_env, sol::load_mode::text);
}

sol::protected_function_result Sandbox::ExecuteString(const std::string& acString) const
{
    return m_pScripting->GetLockedState().Get().script(acString, m_env, sol:: detail::default_chunk_name(), sol::load_mode::text);
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
