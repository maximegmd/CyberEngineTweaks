#include <stdafx.h>

#include "ScriptContext.h"


ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
{
    m_env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if(acName == "onUpdate")
            m_onUpdate = aCallback;
        else if(acName == "onInit")
            m_onInit = aCallback;
    };

    const auto path = acPath / "init.lua";
    const auto result = m_lua.script_file(path.string(), m_env);

    if (result.valid())
    {
        m_initialized = true;
        m_object = result;
    }
}

ScriptContext::~ScriptContext()
{
}

bool ScriptContext::IsValid() const
{
    return m_initialized;
}

void ScriptContext::TriggerOnUpdate() const
{
    if (m_onUpdate)
        m_onUpdate();
}

void ScriptContext::TriggerOnInit() const
{
    if (m_onInit)
        m_onInit();
}

sol::object ScriptContext::GetObject() const
{
    return m_object;
}

