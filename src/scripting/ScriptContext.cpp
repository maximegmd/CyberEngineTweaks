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
        else if(acName == "onToolbarOpen")
            m_onToolbarOpen = aCallback;
        else if(acName == "onToolbarClose")
            m_onToolbarClose = aCallback;
        else
            Logger::WarningToModsFmt("Tried to register unknown handler '{}'!", acName);
    };

    // TODO: proper exception handling!
    try
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
            Logger::ErrorToMods(err.what());
        }
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
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
    // TODO: proper exception handling!
    try
    {
        if (m_onInit)
            m_onInit();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onUpdate)
            m_onUpdate(aDeltaTime);
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

void ScriptContext::TriggerOnDraw() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onDraw)
            m_onDraw();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
    
void ScriptContext::TriggerOnToolbarOpen() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onToolbarOpen)
            m_onToolbarOpen();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
void ScriptContext::TriggerOnToolbarClose() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onToolbarClose)
            m_onToolbarClose();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

sol::object ScriptContext::GetRootObject() const
{
    return m_object;
}

void ScriptContext::TriggerOnShutdown() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onShutdown)
            m_onShutdown();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
