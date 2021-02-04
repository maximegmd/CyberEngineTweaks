#pragma once

#include "LuaSandbox.h"

struct ScriptContext
{
    ScriptContext(LuaSandbox& aLuaSandbox, const std::filesystem::path& acPath, const std::string& acName);
    ScriptContext(ScriptContext&& other) noexcept;
    ~ScriptContext();

    [[nodiscard]] bool IsValid() const;

    const std::vector<VKBindInfo>& GetBinds() const;
    
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetRootObject() const;

private:

    void TriggerOnShutdown() const;

    ScriptContext(const ScriptContext&) = default;
    
    LuaSandbox& m_sandbox;
    size_t m_sandboxID;
    sol::object m_object{ };
    sol::function m_onInit{ };
    sol::function m_onShutdown{ };
    sol::function m_onUpdate{ };
    sol::function m_onDraw{ };
    sol::function m_onOverlayOpen{ };
    sol::function m_onOverlayClose{ };
    std::vector<VKBindInfo> m_vkBindInfos{ };
    std::string m_name{ };
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };
    bool m_initialized{ false };
};