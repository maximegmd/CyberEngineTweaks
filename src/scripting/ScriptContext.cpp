#include <stdafx.h>

#include "ScriptContext.h"
#include "Utils.h"

#include <CET.h>

// TODO: proper exception handling for Lua funcs!
template <typename ...Args>
static sol::protected_function_result TryLuaFunction(std::shared_ptr<spdlog::logger> aLogger, sol::function aFunc, Args... aArgs)
{
    sol::protected_function_result result{ };
    if (aFunc)
    {
        try
        {
            result = aFunc(aArgs...);
        }
        catch(std::exception& e)
        {
            aLogger->error(e.what());
        }
        if (!result.valid())
        {
            const sol::error cError = result;
            aLogger->error(cError.what());
        }
    }
    return result;
}

ScriptContext::ScriptContext(LuaSandbox& aLuaSandbox, const std::filesystem::path& acPath, const std::string& acName)
    : m_sandbox(aLuaSandbox)
    , m_sandboxID(aLuaSandbox.CreateSandbox(acPath, acName))
    , m_name(acName)
{
    auto state = aLuaSandbox.GetState();

    auto& sb = m_sandbox[m_sandboxID];
    auto& env = sb.GetEnvironment();
    m_logger = env["__logger"].get<std::shared_ptr<spdlog::logger>>();

    env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if(acName == "onInit")
            m_onInit = aCallback;
        else if(acName == "onTweak")
            m_onTweak = aCallback;
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

    auto wrapHandler = [&aLuaSandbox, loggerRef = m_logger](sol::function aCallback, const bool acIsHotkey) -> std::variant<std::function<TVKBindHotkeyCallback>, std::function<TVKBindInputCallback>> {
        if (acIsHotkey)
        {
            return [&aLuaSandbox, loggerRef, aCallback]{
                    auto state = aLuaSandbox.GetState();
                    TryLuaFunction(loggerRef, aCallback);
                };
        }

        return [&aLuaSandbox, loggerRef, aCallback](bool isDown){
                auto state = aLuaSandbox.GetState();
                TryLuaFunction(loggerRef, aCallback, isDown);
            };
    };

    auto registerBinding = [this, &wrapHandler](const std::string& acID, const std::string& acDisplayName, const std::string& acDescription, sol::function aCallback, const bool acIsHotkey){
        if (acID.empty() ||
            std::ranges::find_if(acID, [](char c){ return !(isalnum(c) || c == '_' || c == '.'); }) != acID.cend())
        {
            m_logger->error("Tried to register a {} with an incorrect ID format '{}'! ID needs to be alphanumeric without any "
                "whitespace or special characters (exceptions being '_' and '.' which are allowed in ID)!",
                acIsHotkey ? "hotkey" : "input",
                acID);
            return;
        }

        if (acDisplayName.empty())
        {
            m_logger->error("Tried to register a {} with an empty display name! [ID of handler: {}]",
                acIsHotkey ? "hotkey" : "input",
                acID);
            return;
        }

        for (auto& bind : m_vkBinds)
        {
            if (bind.ID == acID)
            {
                m_logger->error("Tried to register a {} with same ID as some other! [ID of handler: {}][Display name of handler: {}]",
                    acIsHotkey ? "hotkey" : "input",
                    acID,
                    acDisplayName);
                return;
            }

            if (bind.DisplayName == acDisplayName)
            {
                m_logger->error("Tried to register a {} with same display name as some other! [ID of handler: {}][Display name of handler: {}]",
                    acIsHotkey ? "hotkey" : "input",
                    acID,
                    acDisplayName);
                return;
            }
        }

        m_vkBinds.emplace_back(acID, acDisplayName, acDescription, wrapHandler(aCallback, acIsHotkey));
    };

    env["registerHotkey"] = sol::overload(
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, const std::string& acDescription, sol::function aCallback)
        {
            registerBinding(acID, acDisplayName, acDescription, aCallback, true);
        },
        [&registerBinding](const std::string& acID, const std::string& acDescription, sol::function aCallback) {
            registerBinding(acID, acDescription, "", aCallback, true);
        });

    env["registerInput"] = sol::overload(
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, const std::string& acDescription, sol::function aCallback) {
            registerBinding(acID, acDisplayName, acDescription, aCallback, false);
        },
        [registerBinding](const std::string& acID, const std::string& acDescription, sol::function aCallback) {
            registerBinding(acID, acDescription, "", aCallback, false);
        });

    // TODO: proper exception handling!
    try
    {
        const auto path = acPath / "init.lua";
        const auto result = sb.ExecuteFile(UTF16ToUTF8(path.native()));

        if (result.valid())
        {
            m_initialized = true;
            m_object = result;
        }
        else
        {
            const sol::error cError = result;
            m_logger->error(cError.what());
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

const VKBind* ScriptContext::GetBind(const std::string& acId) const
{
    const auto it = std::find(m_vkBinds.begin(), m_vkBinds.end(), acId);

    if (it != m_vkBinds.cend())
    {
        return &(*it);
    }

    return nullptr;
}

const TiltedPhoques::Vector<VKBind>& ScriptContext::GetBinds() const
{
    return m_vkBinds;
}

void ScriptContext::TriggerOnTweak() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onTweak);
}

void ScriptContext::TriggerOnInit() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onInit);
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onUpdate, aDeltaTime);
}

void ScriptContext::TriggerOnDraw() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onDraw);
}

void ScriptContext::TriggerOnOverlayOpen() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onOverlayOpen);
}

void ScriptContext::TriggerOnOverlayClose() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onOverlayClose);
}

sol::object ScriptContext::GetRootObject() const
{
    return m_object;
}

void ScriptContext::TriggerOnShutdown() const
{
    auto state = m_sandbox.GetState();

    TryLuaFunction(m_logger, m_onShutdown);
}
