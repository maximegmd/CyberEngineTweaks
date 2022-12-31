#include "stdafx.h"

#include "ScriptContext.h"

#include <CET.h>
#include <Utils.h>

namespace
{
// TODO: proper exception handling for Lua funcs!
template <typename... Args> sol::protected_function_result TryLuaFunction(const std::shared_ptr<spdlog::logger>& acpLogger, const sol::function& aFunc, Args... aArgs)
{
    sol::protected_function_result result{};
    if (aFunc)
    {
        try
        {
            result = aFunc(aArgs...);
        }
        catch (std::exception& e)
        {
            acpLogger->error(e.what());
        }
        if (!result.valid())
        {
            const sol::error cError = result;
            acpLogger->error(cError.what());
        }
    }
    return result;
}

} // namespace

ScriptContext::ScriptContext(LuaSandbox& aLuaSandbox, const std::filesystem::path& acPath, const std::string& acName)
    : m_sandbox(aLuaSandbox)
    , m_sandboxID(aLuaSandbox.CreateSandbox(acPath, acName))
    , m_name(acName)
{
    auto lockedState = aLuaSandbox.GetLockedState();

    auto& sb = m_sandbox[m_sandboxID];
    auto& env = sb.GetEnvironment();
    m_logger = env["__logger"].get<std::shared_ptr<spdlog::logger>>();

    env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if (acName == "onHook")
            m_onHook = aCallback;
        else if (acName == "onTweak")
            m_onTweak = aCallback;
        else if (acName == "onInit")
            m_onInit = aCallback;
        else if (acName == "onShutdown")
            m_onShutdown = aCallback;
        else if (acName == "onUpdate")
            m_onUpdate = aCallback;
        else if (acName == "onDraw")
            m_onDraw = aCallback;
        else if (acName == "onOverlayOpen")
            m_onOverlayOpen = aCallback;
        else if (acName == "onOverlayClose")
            m_onOverlayClose = aCallback;
        else
            m_logger->error("Tried to register an unknown event '{}'!", acName);
    };

    auto wrapHandler = [&aLuaSandbox, loggerRef = m_logger](
                           sol::function aCallback, const bool acIsHotkey) -> std::variant<std::function<TVKBindHotkeyCallback>, std::function<TVKBindInputCallback>>
    {
        if (acIsHotkey)
        {
            return [&aLuaSandbox, loggerRef, aCallback]
            {
                auto lockedState = aLuaSandbox.GetLockedState();

                TryLuaFunction(loggerRef, aCallback);
            };
        }

        return [&aLuaSandbox, loggerRef, aCallback](bool isDown)
        {
            auto lockedState = aLuaSandbox.GetLockedState();

            TryLuaFunction(loggerRef, aCallback, isDown);
        };
    };

    auto wrapDescription = [&aLuaSandbox, loggerRef = m_logger,
                            sandboxId = m_sandboxID](const std::variant<std::string, sol::function>& acDescription) -> std::variant<std::string, std::function<void()>>
    {
        if (std::holds_alternative<sol::function>(acDescription))
        {
            auto callback = std::get<sol::function>(acDescription);
            if (callback != sol::nil)
            {
                return [&aLuaSandbox, loggerRef, callback]
                {
                    auto lockedState = aLuaSandbox.GetLockedState();

                    aLuaSandbox.SetImGuiAvailable(true);

                    const auto previousStyle = ImGui::GetStyle();

                    TryLuaFunction(loggerRef, callback);

                    ImGui::GetStyle() = previousStyle;

                    aLuaSandbox.SetImGuiAvailable(false);
                };
            }
            loggerRef->warn("Tried to register empty tooltip for handler!]");
            return "";
        }
        return std::get<std::string>(acDescription);
    };

    auto registerBinding = [this, &wrapHandler, &wrapDescription](
                               const std::string& acID, const std::string& acDisplayName, const std::variant<std::string, sol::function>& acDescription, sol::function aCallback,
                               const bool acIsHotkey)
    {
        const auto inputTypeStr = acIsHotkey ? "hotkey" : "input";

        if (acID.empty() || std::ranges::find_if(acID, [](char c) { return !(isalnum(c) || c == '_' || c == '.'); }) != acID.cend())
        {
            m_logger->error(
                "Tried to register a {} with an incorrect ID format '{}'! ID needs to be alphanumeric without any "
                "whitespace or special characters (exceptions being '_' and '.' which are allowed in ID)!",
                inputTypeStr, acID);
            return;
        }

        if (acDisplayName.empty())
        {
            m_logger->error("Tried to register a {} with an empty display name! [ID of this handler: {}]", inputTypeStr, acID);
            return;
        }

        const auto bindIt = std::find(m_vkBinds.cbegin(), m_vkBinds.cend(), acID);
        if (bindIt != m_vkBinds.cend())
        {
            m_logger->error(
                "Tried to register a {} with same ID as some other! [ID of this handler: {}, Display name of this "
                "handler: '{}', Display name of other handler: {}]",
                inputTypeStr, acID, acDisplayName, bindIt->DisplayName);
            return;
        }

        m_vkBinds.emplace_back(acID, acDisplayName, wrapDescription(acDescription), wrapHandler(aCallback, acIsHotkey));
    };

    env["registerHotkey"] = sol::overload(
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, sol::function acDescriptionCallback, sol::function aCallback)
        { registerBinding(acID, acDisplayName, acDescriptionCallback, aCallback, true); },
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, const std::string& acDescription, sol::function aCallback)
        { registerBinding(acID, acDisplayName, acDescription, aCallback, true); },
        [&registerBinding](const std::string& acID, const std::string& acDescription, sol::function aCallback) { registerBinding(acID, acDescription, "", aCallback, true); });

    env["registerInput"] = sol::overload(
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, sol::function acDescriptionCallback, sol::function aCallback)
        { registerBinding(acID, acDisplayName, acDescriptionCallback, aCallback, false); },
        [&registerBinding](const std::string& acID, const std::string& acDisplayName, const std::string& acDescription, sol::function aCallback)
        { registerBinding(acID, acDisplayName, acDescription, aCallback, false); },
        [registerBinding](const std::string& acID, const std::string& acDescription, sol::function aCallback) { registerBinding(acID, acDescription, "", aCallback, false); });

    // TODO: proper exception handling!
    try
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(sb.GetRootPath());

        const auto path = GetLuaPath(L"init.lua", acPath, false);

        const auto result = sb.ExecuteFile(UTF16ToUTF8(path.native()));

        current_path(previousCurrentPath);

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
    catch (std::exception& e)
    {
        m_logger->error(e.what());
    }

    env["registerForEvent"] = sol::nil;
    env["registerHotkey"] = sol::nil;
    env["registerInput"] = sol::nil;
}

ScriptContext::ScriptContext(ScriptContext&& other) noexcept
    : ScriptContext(other)
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

void ScriptContext::TriggerOnHook() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onHook);
}

void ScriptContext::TriggerOnTweak() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onTweak);
}

void ScriptContext::TriggerOnInit() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onInit);
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onUpdate, aDeltaTime);
}

void ScriptContext::TriggerOnDraw() const
{
    auto lockedState = m_sandbox.GetLockedState();

    m_sandbox.SetImGuiAvailable(true);

    const auto previousStyle = ImGui::GetStyle();

    TryLuaFunction(m_logger, m_onDraw);

    ImGui::GetStyle() = previousStyle;

    m_sandbox.SetImGuiAvailable(false);
}

void ScriptContext::TriggerOnOverlayOpen() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onOverlayOpen);
}

void ScriptContext::TriggerOnOverlayClose() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onOverlayClose);
}

sol::object ScriptContext::GetRootObject() const
{
    return m_object;
}

void ScriptContext::TriggerOnShutdown() const
{
    auto lockedState = m_sandbox.GetLockedState();

    TryLuaFunction(m_logger, m_onShutdown);
}
