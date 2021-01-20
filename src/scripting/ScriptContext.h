#pragma once

struct ScriptContext
{
    ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath);
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

    sol::state_view m_lua;
    sol::environment m_env;
    sol::object m_object{ };
    sol::function m_onInit{ };
    sol::function m_onShutdown{ };
    sol::function m_onUpdate{ };
    sol::function m_onDraw{ };
    sol::function m_onOverlayOpen{ };
    sol::function m_onOverlayClose{ };
    std::vector<VKBindInfo> m_vkBindInfos{ };
    std::filesystem::path m_path{ };
    std::string m_name{ };
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };
    bool m_initialized{ false };
};