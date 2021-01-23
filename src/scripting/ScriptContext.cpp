#include <stdafx.h>

#include "ScriptContext.h"

#include <Utils.h>

ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
    , m_name(acPath.string())
{
    // initialize logger for this mod
    m_logger = CreateLogger(acPath / (m_name + ".log"), "mods." + m_name);
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
        else if(acName == "onOverlayOpen")
            m_onOverlayOpen = aCallback;
        else if(acName == "onOverlayClose")
            m_onOverlayClose = aCallback;
        else
            m_logger->error("Tried to register an unknown event '{}'!", acName);
    };

    m_env["registerHotkey"] = [this](const std::string& acID, const std::string& acDescription, sol::function aCallback)
    {
        if (acID.empty() ||
            (std::find_if(acID.cbegin(), acID.cend(), [](char c){ return !(isalpha(c) || isdigit(c) || c == '_'); }) != acID.cend()))
        {
            m_logger->error("Tried to register a hotkey with an incorrect ID format '{}'! ID needs to be alphanumeric without any whitespace or special characters (exception being '_' which is allowed in ID)!", acID);
            return;
        }

        if (acDescription.empty())
        {
            m_logger->error("Tried to register a hotkey with an empty description! (ID of hotkey handler: {})", acID);
            return;
        }

        auto loggerRef = m_logger;
        std::string vkBindID = m_name + '.' + acID;
        VKBind vkBind = { vkBindID, acDescription, [loggerRef, aCallback]()
        {
            // TODO: proper exception handling!
            try
            {
                if (aCallback)
                    aCallback();
            }
            catch(std::exception& e)
            {
                loggerRef->error(e.what());
            }
        }};

        m_vkBindInfos.emplace_back(VKBindInfo{vkBind});
    };

    // assign logger to mod so it can be used from within it too
    {
        auto loggerRef = m_logger; // ref for lambdas
        sol::table spdlog(m_lua, sol::create);
        spdlog["trace"] = [loggerRef](const std::string& message)
        {
            loggerRef->trace(message);
        };
        spdlog["debug"] = [loggerRef](const std::string& message)
        {
            loggerRef->debug(message);
        };
        spdlog["warning"] = [loggerRef](const std::string& message)
        {
            loggerRef->warn(message);
        };
        spdlog["error"] = [loggerRef](const std::string& message)
        {
            loggerRef->error(message);
        };
        spdlog["critical"] = [loggerRef](const std::string& message)
        {
            loggerRef->critical(message);
        };
        m_env["spdlog"] = spdlog;
    }

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
            m_logger->error(err.what());
        }
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
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

    m_logger->flush();
}

bool ScriptContext::IsValid() const
{
    return m_initialized;
}

const std::vector<VKBindInfo>& ScriptContext::GetBinds() const
{
    return m_vkBindInfos;
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
        m_logger->error(e.what());
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
        m_logger->error(e.what());
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
        m_logger->error(e.what());
    }
}
    
void ScriptContext::TriggerOnOverlayOpen() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayOpen)
            m_onOverlayOpen();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}
void ScriptContext::TriggerOnOverlayClose() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayClose)
            m_onOverlayClose();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
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
        m_logger->error(e.what());
    }
}
