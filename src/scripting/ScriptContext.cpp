#include <stdafx.h>

#include "ScriptContext.h"


ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
{
    m_env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if(acName == "onInit")
            m_onInit = aCallback;
        else if(acName == "onShutdown")
            m_onShutdown = aCallback;
        else if(acName == "onUpdate")
            m_onUpdate = aCallback;
        else if(acName == "onDraw")
            m_onDraw = aCallback;
    };

    const auto path = acPath / "init.lua";
    const auto result = m_lua.script_file(path.string(), m_env);

    if (result.valid())
    {
        m_initialized = true;
        m_object = result;
    }
    else
    {
        sol::error err = result;
        std::string what = err.what();
        spdlog::error(what);
    }
}

ScriptContext::~ScriptContext()
{
    if (m_initialized)
        TriggerOnShutdown();
}

bool ScriptContext::IsValid() const
{
    return m_initialized;
}

void ScriptContext::TriggerOnInit() const
{
    if (m_onInit)
        m_onInit();
}

void ScriptContext::TriggerOnShutdown() const
{
    if (m_onShutdown)
        m_onShutdown();
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    if (m_onUpdate)
        m_onUpdate(aDeltaTime);
}

void ScriptContext::TriggerOnDraw() const
{
    if (m_onDraw)
        m_onDraw();
}

sol::object ScriptContext::GetObject() const
{
    return m_object;
}

