#include <stdafx.h>

#include "ScriptContext.h"

ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
    , m_name(std::filesystem::relative(acPath, Paths::ModsPath).string())
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
            Logger::ErrorToModsFmt("Tried to register unknown handler '{}'!", acName);
    };

    m_env["registerVKBind"] = [this](const std::string& acID, const std::string& acDescription, sol::table aVKBindCode, sol::function aCallback)
    {
        if (acID.empty() ||
            (std::find_if(acID.cbegin(), acID.cend(), [](char c){ return !(isalpha(c) || isdigit(c) || c == '_'); }) != acID.cend()))
        {
            Logger::ErrorToModsFmt("Tried to register VKBind with incorrect ID format '{}'! ID needs to be alphanumeric without any whitespace or special characters ('_' excluded)!", acID);
            return;
        }

        if (acDescription.empty())
        {
            Logger::ErrorToModsFmt("Tried to register VKBind with empty description!! (ID of VKBind handler: {})", acID);
            return;
        }

        if (aVKBindCode.size() > 4)
        {
            Logger::ErrorToModsFmt("Tried to register VKBind with too many keys! Maximum 4-key combos allowed! (ID of VKBind handler: {})", acID);
            return;
        }
        
        std::string vkBindID = m_name + '.' + acID;
        VKCodeBindDecoded vkCodeBindDec{ };
        for (size_t i = 0; i < 4; ++i)
        {
            auto codePart = aVKBindCode[i + 1];
            if (codePart.valid())
                vkCodeBindDec[i] = static_cast<uint8_t>(codePart.get_or<UINT>(0));
        }
        VKBind vkBind = { vkBindID, acDescription, [aCallback]()
        {
            // TODO: proper exception handling!
            try
            {
                if (aCallback)
                    aCallback();
            }
            catch(std::exception& e)
            {
                Logger::ErrorToMods(e.what());
            }
        }};
        UINT vkCodeBind = VKBindings::EncodeVKCodeBind(vkCodeBindDec);
        m_vkBindInfos.emplace_back(VKBindInfo{vkBind, vkCodeBind});
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

std::vector<VKBindInfo>& ScriptContext::GetBinds()
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
