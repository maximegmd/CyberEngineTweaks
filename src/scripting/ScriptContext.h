#pragma once

#ifdef SOL_NO_EXCEPTIONS
  #define CET_SOL_SAFE_EXEC(commands, handler) commands
#else
  #define CET_SOL_SAFE_EXEC(commands, handler) try commands catch(std::exception& e) handler
#endif // SOL_NO_EXCEPTIONS

struct ScriptContext
{
    ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath);
    ScriptContext(ScriptContext&& other) noexcept;
    ~ScriptContext();

    [[nodiscard]] bool IsValid() const;
    
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;

    sol::object Object() const;

protected:

    void TriggerOnShutdown() const;

private:

    ScriptContext(const ScriptContext&) = default;

    sol::state_view m_lua;
    sol::environment m_env;
    sol::object m_object;
    sol::function m_onInit;
    sol::function m_onShutdown;
    sol::function m_onUpdate;
    sol::function m_onDraw;
    bool m_initialized{false};
};