#include <stdafx.h>

#include "ScriptContext.h"

#include <CET.h>

ScriptContext::ScriptContext(LuaSandbox& aLuaSandbox, const std::filesystem::path& acPath, const std::string& acName)
    : m_sandbox(aLuaSandbox)
    , m_sandboxID(aLuaSandbox.CreateSandbox(acPath))
    , m_name(acName)
{
    auto& sb = m_sandbox[m_sandboxID];
    auto& env = sb.GetEnvironment();
    m_logger = m_sandbox.InitializeLoggerForEnvironment(env, acPath / (acName + ".log"), acName);

    env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
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

    env["registerHotkey"] = [this](const std::string& acID, const std::string& acDescription, sol::function aCallback)
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
                {
                    auto res = aCallback();
                    if (!res.valid())
                    {
                        sol::error err = res;
                        loggerRef->error(err.what());
                    }
                }
            }
            catch(std::exception& e)
            {
                loggerRef->error(e.what());
            }
        }};

        m_vkBindInfos.emplace_back(VKBindInfo{vkBind});
    };    

    // TODO: proper exception handling!
    try
    {
        const auto path = acPath / "init.lua";
        const auto result = sb.ExecuteFile(path.string());

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
        {
            auto res = m_onInit();
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
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
        {
            auto res = m_onUpdate(aDeltaTime);
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
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
        {
            auto res = m_onDraw();
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
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
        {
            auto res = m_onOverlayOpen();
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
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
        {
            auto res = m_onOverlayClose();
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
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
        {
            auto res = m_onShutdown();
            if (!res.valid())
            {
                sol::error err = res;
                m_logger->error(err.what());
            }
        }
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}
