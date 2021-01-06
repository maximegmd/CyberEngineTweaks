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

    CET_SOL_SAFE_EXEC
    (
        // try
        {
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
        },
        // catch
        {
            std::string what = e.what();
            spdlog::error(what);
        }
    )
}

ScriptContext::ScriptContext(ScriptContext&& other) noexcept : ScriptContext(other)
{
    other.m_initialized = false;
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
    CET_SOL_SAFE_EXEC
    (
        // try
        {
            if (m_onInit)
                m_onInit();
        },
        // catch
        {
            std::string what = e.what();
            spdlog::error(what);
        }
    )
}

void ScriptContext::TriggerOnShutdown() const
{
    CET_SOL_SAFE_EXEC
    (
        // try
        {
            if (m_onShutdown)
                m_onShutdown();
        },
        // catch
        {
            std::string what = e.what();
            spdlog::error(what);
        }
    )
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    CET_SOL_SAFE_EXEC
    (
        // try
        {
            if (m_onUpdate)
                m_onUpdate(aDeltaTime);
        },
        // catch
        {
            std::string what = e.what();
            spdlog::error(what);
        }
    )
}

void ScriptContext::TriggerOnDraw() const
{
    CET_SOL_SAFE_EXEC
    (
        // try
        {
            if (m_onDraw)
                m_onDraw();
        },
        // catch
        {
            std::string what = e.what();
            spdlog::error(what);
        }
    )
}

sol::object ScriptContext::Object() const
{
    return m_object;
}

