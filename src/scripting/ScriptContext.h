#pragma once

#include "LuaSandbox.h"

struct LogWindow;
struct ScriptContext
{
    ScriptContext(LuaSandbox& aLuaSandbox, const std::filesystem::path& acPath, const std::string& acName);
    ScriptContext(ScriptContext&& other) noexcept;
    ~ScriptContext();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] const VKBind* GetBind(const std::string& acId) const;
    [[nodiscard]] const TiltedPhoques::Vector<VKBind>& GetBinds() const;

    void TriggerOnHook() const;
    void TriggerOnTweak() const;
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw();

    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    [[nodiscard]] sol::object GetRootObject() const;

private:

    void TriggerOnShutdown() const;

    ScriptContext(const ScriptContext&) = default;

    LuaSandbox& m_sandbox;
    uint64_t m_sandboxID;
    sol::object m_object{ };
    sol::function m_onHook{ };
    sol::function m_onTweak{ };
    sol::function m_onInit{ };
    sol::function m_onShutdown{ };
    sol::function m_onUpdate{ };
    sol::function m_onDraw{ };
    sol::function m_onOverlayOpen{ };
    sol::function m_onOverlayClose{ };
    TiltedPhoques::Vector<VKBind> m_vkBinds{ };
    std::string m_name{ };
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };
    std::shared_ptr<LogWindow> m_loggerWindow{ nullptr };
    bool m_initialized{ false };
};